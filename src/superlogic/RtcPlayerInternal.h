#ifndef LF_RTP_PLAYER_H
#define LF_RTP_PLAYER_H

#include "../engine_api/RtcPlayer.h"
#include "../engine_api/rtp_api.h"
#include "../superlogic/webrtc_base.h"
#include "../sdk/sdk_interface.h"
#include "../superlogic/avengine_api.h"
#include <stdint.h>
#include <string>
#include <pthread.h>
#if defined(__ANDROID__)
#include <jni.h>
#endif

namespace live_stream_sdk {
  class RTPDownload;
}

class RtcPlayerInternal : public WebrtcBase, public live_stream_sdk::RtpNetworkObserver {
public:
  RtcPlayerInternal(RtcPlayer* owner, const char *deviceid);
  ~RtcPlayerInternal();
  int Start(const RtcPlayer::NetworkConfig* net, RtcPlayer::RtcNetworkCallback callback);
  void Mute(bool mute);
  void SetNetworkChanged();
  void SetDecodedVideoCallback(RtcPlayer::RtcDecodedVideoCallback callback);
  int Stop();
  int SetWindow(void* hwnd);
  RtcPlayerState GetState();
  live_stream_sdk::RTPDownload *GetDownloader();
  int Snapshot(const char *path);

public:
#if defined(__ANDROID__)
  static JavaVM* jvm_;
#endif

private:
  virtual void DecodedVideoCallbackImpl(char* data[3], lfrtcRawVideoType type, int width, int height);
  virtual void WebrtcNotifyCallbackImpl(uint32_t msgid, int wParam, int lParam);
  virtual void WebrtcReportCallbackImpl(const char *msg);
  virtual void OnNetworkState(int state_no, int wParam, int lParam);
  virtual void OnReceivedRTCP(const unsigned char* data, unsigned int len, int type);
  virtual void OnReceivedRTP(unsigned char* data, unsigned int len, int type);

  void HandleSnapshot(char* yuvbuf[3], int wd, int ht);

  int StartDec();
  int StopDec();

  enum RTPPlayerState {
    RTPPlayerConstruct = 0,
    RTPPlayerInit,
    RTPPlayerRunning,
    RTPPlayerReseting,
  };

  live_stream_sdk::RTPDownload* m_download;
  //RTPPlayerState state_;
  lfrtcAVID m_audio_channel;
  lfrtcAVID m_video_channel;
  bool m_snapshot;
  std::string m_snapshot_path;
  pthread_mutex_t m_mutex;;
  void *m_window;

  RtcPlayer::RtcNetworkCallback m_notify_callback;
  RtcPlayer::RtcDecodedVideoCallback m_decoded_video_callback;
  RtcPlayer *m_owner;
  bool m_mute;
};

#endif
