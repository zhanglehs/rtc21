#include "../superlogic/RtcPlayerInternal.h"

#include "../log/log.h"
#include "../engine_api/rtp_download.h"
#include <assert.h>

extern "C" {
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
#include <stdint.h>
}

#ifdef WIN32
#pragma comment(lib, "avengine_dll.dll.lib")
#endif

using namespace live_stream_sdk;

namespace {

#if defined(__ANDROID__)
class CLocalJniEnv {
public:
  CLocalJniEnv(JavaVM* jvm) {
    m_jvm = jvm;
    m_env = NULL;
    m_attached = false;

    if (m_jvm->GetEnv((void**)&m_env, JNI_VERSION_1_4) != JNI_OK) {
      jint res = jvm->AttachCurrentThread(&m_env, NULL);
      if ((res < 0) || !m_env) {
        // ERR("Could not attach thread from JVM (%d, %p)", res, env);
        m_env = NULL;
        return;
      }
      m_attached = true;
    }
  }

  ~CLocalJniEnv() {
    if (m_attached) {
      if (m_jvm->DetachCurrentThread() < 0) {
        // ERR("Could not detach thread from JVM");
      }
    }
  }

  JNIEnv* getEnv() {
    return m_env;
  }

private:
  JavaVM* m_jvm;
  JNIEnv* m_env;
  bool m_attached;
};
#endif
}

RtcPlayerInternal::RtcPlayerInternal(RtcPlayer* owner, const char *deviceid) {
  m_audio_channel = NULL;
  m_video_channel = NULL;
  m_snapshot = false;
  m_window = NULL;
  pthread_mutex_init(&m_mutex, NULL);

  m_download = new RTPDownload(deviceid, this);
  m_notify_callback = NULL;
  m_decoded_video_callback = NULL;
  m_owner = owner;
  m_mute = false;
}

RtcPlayerInternal::~RtcPlayerInternal() {
  Stop();
  delete m_download;
  pthread_mutex_destroy(&m_mutex);
}

int RtcPlayerInternal::Start(const RtcPlayer::NetworkConfig* net, RtcPlayer::RtcNetworkCallback callback) {
  m_download->Stop();
  StopDec();

  m_notify_callback = callback;

  if (StartDec() < 0) {
    return -1;
  }

  rtcDownloadDispatchConfig config;
  strcpy(config.alias, net->alias);
  strcpy(config.appid, net->appid);
  strcpy(config.lapi_host, net->lapi);
  strcpy(config.lapi_token, net->token);
  if (m_download->Start(&config) < 0) {
    return -1;
  }

  SetWindow(m_window);

  return 0;
}

int RtcPlayerInternal::SetWindow(void* hwnd) {
#if defined(__ANDROID__)
  if (hwnd || m_window) {
    CLocalJniEnv localEnv(RtcPlayerInternal::jvm_);
    JNIEnv* env = localEnv.getEnv();
    if (!env) {
      return -1;
    }
    if (hwnd) {
      hwnd = (void *)env->NewGlobalRef((jobject)hwnd);
    }
    if (m_window) {
      env->DeleteGlobalRef((jobject)m_window);
      m_window = NULL;
    }
  }
#endif
  m_window = hwnd;
  if (m_video_channel) {
    return lfrtcSetPlayWindow(hwnd, m_video_channel);
  }
  return 0;
}

int RtcPlayerInternal::Stop() {
    m_download->Stop();
    StopDec();
    m_mute = false;

#if defined(__ANDROID__)
  if (m_window) {
    CLocalJniEnv localEnv(RtcPlayerInternal::jvm_);
    JNIEnv* env = localEnv.getEnv();
    if (!env) {
      return -1;
    }
    env->DeleteGlobalRef((jobject)m_window);
  }
#endif
  m_window = NULL;
  return 0;
}

int RtcPlayerInternal::Snapshot(const char *path) {
  if (path == NULL || path[0] == 0
    || GetState() != RtcPlayerState::RTC_PLAYER_STATE_RUNNING) {
    return -1;
  }
  pthread_mutex_lock(&m_mutex);
  m_snapshot = true;
  m_snapshot_path = path;
  pthread_mutex_unlock(&m_mutex);
  return 0;
}

void RtcPlayerInternal::HandleSnapshot(char* yuvbuf[3], int wd, int ht) {
  if (m_snapshot) {
    std::string path;
    pthread_mutex_lock(&m_mutex);
    if (m_snapshot) {
      path = m_snapshot_path;
      m_snapshot = false;
    }
    pthread_mutex_unlock(&m_mutex);

    if (path.empty()) {
      return;
    }
    int ret = lfrtcSaveFrameToJPEG((unsigned char**)yuvbuf, wd, ht, path.c_str());
    if (ret < 0) {
      ERR("SnapShot error ret %d", ret);
    }
    if (m_notify_callback) {
      m_notify_callback(m_owner, RtcPlayerMsgid::RTC_PLAYER_MSG_VIDEO_SNAPSHOT, (long)path.c_str(), long(ret < 0));
    }
  }
}

void RtcPlayerInternal::DecodedVideoCallbackImpl(char* data[3], lfrtcRawVideoType type, int width, int height) {
  HandleSnapshot(data, width, height);
  if (m_decoded_video_callback) {
    m_decoded_video_callback(m_owner, data, type, width, height);
  }
}

void RtcPlayerInternal::WebrtcNotifyCallbackImpl(uint32_t msgid, int wParam, int lParam) {
  switch (msgid) {
  case LFRTC_VIDEO_GET_RESOLUTION:
    INF("resize widht %d height %d", wParam, lParam);
    if (m_notify_callback) {
      m_notify_callback(m_owner, RtcPlayerMsgid::RTC_PLAYER_MSG_VIDEO_RESOLUTION, wParam, lParam);
    }
    break;
  case LFRTC_VIDEO_FIRST_FRAME:
    // TODO: zhangle, android
    if (m_notify_callback) {
      m_notify_callback(m_owner, RtcPlayerMsgid::RTC_PLAYER_MSG_VIDEO_FIRST_FRAME, 0, 0);
    }
    break;
  default:
    break;
  }
}

void RtcPlayerInternal::WebrtcReportCallbackImpl(const char *msg) {
  // TODO: zhangle, http report
  //Json::Reader reader;
  //Json::Value jmsg;
  //reader.parse(msg, msg + strlen(msg), jmsg, false);
}

void RtcPlayerInternal::OnNetworkState(int state_no, int wParam, int lParam) {
  if (m_notify_callback) {
    m_notify_callback(m_owner, state_no, wParam, lParam);
  }
}

void RtcPlayerInternal::OnReceivedRTCP(const unsigned char* data, unsigned int len, int type) {
  if (type == 97) {
    lfrtcSetAudioRtcp(data, len, m_audio_channel);
  }
  else if (type == 96) {
    lfrtcSetVideoRtcp(data, len, m_video_channel);
  }
}

void RtcPlayerInternal::OnReceivedRTP(unsigned char* data, unsigned int len, int type) {
  if (len < 24) {
    ERR("len too small %d", len);
    return;
  }

  if (type == 97 && !m_mute) {
    char *buf2 = new char[len];
    memcpy(buf2, data, len);
    lfrtcSetRtpData((void*)buf2, len, m_audio_channel);
    delete[] buf2;
  }
  else if (type == 96) {
    lfrtcSetRtpData((void*)data, len, m_video_channel);
  }
}

int RtcPlayerInternal::StartDec() {
  if (m_audio_channel && m_video_channel) {
    return 0;
  }
  StopDec();

  lfrtcDevice AudioDevices[3];
  lfrtcGetAudioPlayoutDevice(AudioDevices, sizeof(AudioDevices)/sizeof(AudioDevices[0]));
  lfrtcSetAudioPlayoutDevice(AudioDevices[0].szDeviceID);
  lfrtcStartPlay(this, m_audio_channel, m_video_channel);
  if (m_audio_channel == NULL || m_video_channel == NULL) {
    ERR("StartDec failed");
    StopDec();
    return -1;
  }
  return 0;
}

int RtcPlayerInternal::StopDec() {
  INF("StopDec()");
  int ret = lfrtcStopPlay(m_audio_channel, m_video_channel);
  m_audio_channel = NULL;
  m_video_channel = NULL;
  return ret;
}

void RtcPlayerInternal::Mute(bool mute) {
  m_mute = mute;
  // TODO: zhangle, notify webrtc
}

void RtcPlayerInternal::SetNetworkChanged() {
  m_download->SetNetworkChanged();
}

void RtcPlayerInternal::SetDecodedVideoCallback(RtcPlayer::RtcDecodedVideoCallback callback) {
  m_decoded_video_callback = callback;
}

RtcPlayerState RtcPlayerInternal::GetState() {
  return m_download->GetState();
}

live_stream_sdk::RTPDownload * RtcPlayerInternal::GetDownloader() {
  return m_download;
}

#if defined(__ANDROID__)
JavaVM* RtcPlayerInternal::jvm_ = NULL;
#endif
