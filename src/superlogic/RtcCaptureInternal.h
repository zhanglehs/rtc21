// edit by zhangle
#ifndef LF_RTC_CAPTURE_INTERNAL_H
#define LF_RTC_CAPTURE_INTERNAL_H

#include "../engine_api/RtcCapture.h"
#include "../superlogic/avengine_api.h"
#include "../superlogic/webrtc_base.h"
#include "../sdk/sdk_interface.h"
#include <string>

class RtcCaptureInternal : public WebrtcBase, public live_stream_sdk::RtpNetworkObserver {
public:
  RtcCaptureInternal(RtcCapture* owner, const char* deviceid);
  ~RtcCaptureInternal();

  int StartCapture(const lfrtcCaptureConfig* config);
  int StartPreview(void *window);
  int StartPreview(T_lfrtcPreviewVideoCb callback, lfrtcRawVideoType videotype);
  int StartEncodeAndSend(const RtcCapture::NetworkConfig* net, const lfrtcEncodeConfig* encode, RtcCapture::RtcNetworkCallback callback);
  void StopEncodeAndSend();
  void StopPreview();
  void Stop();
  RtcCaptureState GetState();

  void GetAdvancedStateInfo(RtcCapture::AdvancedStateInfo *info);
  live_stream_sdk::RTPUpload* GetUploader();
  void EnableNoiseSuppression(bool enable);
  void EnableEchoCancellation(bool enable);

private:
  virtual void EncodedVideoCallbackImpl(char* data, int len, unsigned int timestamp);
  virtual void EncodedAudioCallbackImpl(char* data, int len, unsigned int timestamp);
  virtual void OnReceivedRTCP(const unsigned char* data, unsigned int len, int type);
  virtual void OnNetworkState(int state_no, int wParam, int lParam);

  RtcCapture* m_owner;
  live_stream_sdk::RTPUpload* m_upload;
  lfrtcAVID m_capture_id;
  lfrtcAVID m_audio_channel_id;
  lfrtcAVID m_video_channel_id;
  char* m_video_rtp;
  char* m_audio_rtp;
  int m_width;
  int m_height;
  unsigned int m_first_video_timestamp;
  unsigned int m_first_audio_timestamp;
  bool m_inited_audio_timestamp;
  bool m_inited_video_timestamp;
  RtcCapture::RtcNetworkCallback m_net_callback;
  std::string m_deviceid;
  bool m_enable_ans;
  bool m_enable_aec;
};

#endif
