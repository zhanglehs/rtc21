// edit by zhangle
#include "../superlogic/RtcCaptureInternal.h"

#include "../superlogic/avengine_api.h"
#include "../log/log.h"
#include "../engine_api/rtp_upload.h"
#include "../engine_api/rtp_api.h"
#include "../avformat/rtcp.h"
#include <assert.h>
#include <time.h>

#ifdef WIN32
#pragma comment(lib, "avengine_dll.dll.lib")
#endif

RtcCaptureInternal::RtcCaptureInternal(RtcCapture* owner, const char* deviceid) {
  m_owner = owner;
  m_upload = new live_stream_sdk::RTPUpload(deviceid, this);
  m_capture_id = NULL;
  m_audio_channel_id = NULL;
  m_video_channel_id = NULL;
  m_video_rtp = new char[2 * 1024 * 1024];
  m_audio_rtp = new char[2048 * 4];
  m_width = 0;
  m_height = 0;
  m_first_video_timestamp = 0;
  m_first_audio_timestamp = 0;
  m_inited_audio_timestamp = false;
  m_inited_video_timestamp = false;
  m_net_callback = NULL;
  m_enable_ans = true;
  m_enable_aec = true;
}

RtcCaptureInternal::~RtcCaptureInternal() {
  Stop();
  delete[] m_video_rtp;
  delete[] m_audio_rtp;
  delete m_upload;
}

int RtcCaptureInternal::StartCapture(const lfrtcCaptureConfig* config) {
  if (m_capture_id) {
    return 0;
  }
  lfrtcEnableNoiseSuppression(m_enable_ans);
  lfrtcEnableAEC(m_enable_aec);
  return lfrtcStartCapture(config, m_capture_id);
}

int RtcCaptureInternal::StartPreview(void *window) {
  return lfrtcStartPreview(window, m_capture_id);
}

int RtcCaptureInternal::StartPreview(T_lfrtcPreviewVideoCb callback, lfrtcRawVideoType videotype) {
  return lfrtcStartPreview2(m_capture_id, callback, videotype, m_owner);
}

int RtcCaptureInternal::StartEncodeAndSend(const RtcCapture::NetworkConfig* net, const lfrtcEncodeConfig* encode, RtcCapture::RtcNetworkCallback callback) {
  if (m_capture_id == NULL) {
    return -1;
  }
  if (m_video_channel_id || m_video_channel_id) {
    return -1;
  }

  m_net_callback = callback;
  live_stream_sdk::rtcUploadDispatchConfig dispatch;
  strcpy(dispatch.alias, net->alias);
  strcpy(dispatch.lapi_host, net->lapi);
  strcpy(dispatch.lapi_token, net->token);
  strcpy(dispatch.appid, net->appid);
  if (m_upload->Start(&dispatch) < 0) {
    return -1;
  }

  m_width = encode->video_encode_width;
  m_height = encode->video_encode_height;

  unsigned int audio_ssrc = 0;
  unsigned int video_ssrc = 0;
  m_upload->GetSSRC(audio_ssrc, video_ssrc);

  int ret = lfrtcStartEncodeAndSend(m_capture_id, encode, m_audio_channel_id, m_video_channel_id, this);
  if (ret < 0) {
    m_upload->Stop();
    return ret;
  }
  lfrtcSetVideoSSRC(video_ssrc, m_video_channel_id);
  lfrtcSetAudioSSRC(audio_ssrc, m_audio_channel_id);
  return ret;
}

void RtcCaptureInternal::StopEncodeAndSend() {
  lfrtcStopEncodeAndSend(m_audio_channel_id, m_video_channel_id);
  m_audio_channel_id = NULL;
  m_video_channel_id = NULL;
  m_inited_audio_timestamp = false;
  m_inited_video_timestamp = false;
  m_upload->Stop();
}

void RtcCaptureInternal::Stop() {
  StopEncodeAndSend();
  lfrtcStopCapture(m_capture_id);
  m_capture_id = NULL;
}

void RtcCaptureInternal::GetAdvancedStateInfo(RtcCapture::AdvancedStateInfo *info) {
  lfrtcGetVideoEncodeInfo(m_video_channel_id, info->video_encode_fps, info->video_encode_bitrate);
  info->video_target_bitrate = lfrtcGetVideoTargetEncodeBitrate(m_video_channel_id);
}

live_stream_sdk::RTPUpload* RtcCaptureInternal::GetUploader() {
  return m_upload;
}

void RtcCaptureInternal::StopPreview() {
  lfrtcStopPreview(m_capture_id);
}

void RtcCaptureInternal::EncodedVideoCallbackImpl(char* data, int len, unsigned int timestamp) {
  if (NULL == data || len <= 0 || len > 1024 * 1024) {
    ERR("EncodedVideoCallback error: data=%p, len=%d, timestamp=%u",
      data, len, timestamp);
    return;
  }

  if (!m_inited_video_timestamp) {
    m_inited_video_timestamp = true;
    m_first_video_timestamp = timestamp;
  }
  timestamp -= m_first_video_timestamp;
  m_upload->SendH264(data, len, m_width, m_height, timestamp / 90);
}

void RtcCaptureInternal::EncodedAudioCallbackImpl(char* data, int len, unsigned int timestamp) {
  timestamp = (unsigned int)avformat::Clock::GetRealTimeClock()->TimeInMilliseconds();
  if (NULL == data || len <= 0 || len > 2048 * 4) {
    ERR("EncodedVideoCallback error: data=%p, len=%d, timestamp=%u",
      data, len, timestamp);
    return;
  }
  
  if (!m_inited_audio_timestamp) {
    m_inited_audio_timestamp = true;
    m_first_audio_timestamp = timestamp;
  }
  timestamp -= m_first_audio_timestamp;
  m_upload->SendAAC(data, len, timestamp);
}

void RtcCaptureInternal::OnReceivedRTCP(const unsigned char* data, unsigned int len, int type) {
  if (type == 96) {
    lfrtcSetVideoRtcp(data, len, m_video_channel_id);
  }
  else if (type == 97) {
    lfrtcSetAudioRtcp(data, len, m_audio_channel_id);
  }
}

void RtcCaptureInternal::EnableNoiseSuppression(bool enable) {
  m_enable_ans = enable;
  lfrtcEnableNoiseSuppression(m_enable_ans);
}

void RtcCaptureInternal::EnableEchoCancellation(bool enable) {
  m_enable_aec = enable;
  lfrtcEnableAEC(m_enable_aec);
}

void RtcCaptureInternal::OnNetworkState(int state_no, int wParam, int lParam) {
  if (m_net_callback) {
    m_net_callback(m_owner, state_no, wParam, lParam);
  }
}

RtcCaptureState RtcCaptureInternal::GetState() {
  return m_upload->GetState();
}
