// edit by zhangle
#ifndef LF_RTC_CAPTURE_H
#define LF_RTC_CAPTURE_H

#include "avengine_types.h"

#ifndef DLLEXPORT
#ifdef WIN32
#ifdef ENGINE_SDK_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif
#else
#define DLLEXPORT
#endif
#endif

enum RtcCaptureErrorType {
  RTC_CAPTURE_ERROR_CREATETHREAD_FAILED = 100,
  RTC_CAPTURE_ERROR_INVALID_ALIAS,
  RTC_CAPTURE_ERROR_NETWORK_TIMEOUT,
  RTC_CAPTURE_ERROR_NETWORK_ERROR,
  RTC_CAPTURE_ERROR_GETUPLOADURL_FAILED,
  RTC_CAPTURE_ERROR_GETMCUINFO_FAILED,
  RTC_CAPTURE_ERROR_SENDSDP_FAILED,
  RTC_CAPTURE_ERROR_MCU_DENY,
};

enum RtcCaptureStopType {
  RTC_CAPTURE_STOP_BY_ERROR = 100,
  //RTC_CAPTURE_STOP_BY_USER,
};

enum RtcCaptureState {
  RTC_CAPTURE_STATE_UNSET = 100,
  RTC_CAPTURE_STATE_INITIALIZING,
  RTC_CAPTURE_STATE_RUNNING,
  // wParam中会带上失败的原因RtcCaptureErrorType。
  // 处于RTC_CAPTURE_ERROR状态时内部会自动重试，重试成功后会发送RTC_CAPTURE_STATE_RUNNING；
  // 如果确定不再重试了，会再次发送RTC_CAPTURE_STATE_STOPPED消息
  RTC_CAPTURE_STATE_ERROR,
  // wParam中会带上stop的原因RtcCaptureStopType
  // 用户自己调用Stop函数导致的stop并不会发送RTC_CAPTURE_STATE_STOPPED回调通知
  RTC_CAPTURE_STATE_STOPPED,
};

namespace live_stream_sdk {
  class RTPUpload;
}

class RtcCaptureInternal;

class DLLEXPORT RtcCapture {
public:
  typedef void(*RtcNetworkCallback)(RtcCapture* ctx, unsigned int msgid, int wParam, int lParam);

  struct AdvancedStateInfo {
    unsigned int video_encode_fps;
    unsigned int video_encode_bitrate;
    unsigned int video_target_bitrate;
  };

  struct NetworkConfig {
    char lapi[256];
    char appid[256];
    char alias[256];
    char token[256];
    NetworkConfig() {
      lapi[0] = 0;
      appid[0] = 0;
      alias[0] = 0;
      token[0] = 0;
    }
  };

public:
  RtcCapture(const char* deviceid);
  ~RtcCapture();

  int StartCapture(const lfrtcCaptureConfig* config);
  int StartPreview(void *window);
  int StartPreview(T_lfrtcPreviewVideoCb callback, lfrtcRawVideoType videotype);
  int StartEncodeAndSend(const NetworkConfig* net, const lfrtcEncodeConfig* encode, RtcNetworkCallback callback = 0);
  void StopEncodeAndSend();
  void StopPreview();
  void Stop();
  RtcCaptureState GetState();

  static int GetAudioRecorderDevice(lfrtcDevice* buf, int count);
  static int GetCameraDevice(lfrtcDevice* buf, int count);
  static int GetCameraCapability(const char* deviceid, lfrtcCameraCapability* buf, int count);

  void SetUserdata(void *userdata) { m_userdata = userdata; }
  void *GetUserdata() { return m_userdata; }

  // 以下为高级用法
  void GetAdvancedStateInfo(AdvancedStateInfo *info);
  live_stream_sdk::RTPUpload* GetUploader();
  // 躁声抑制开关，默认为true
  void EnableNoiseSuppression(bool enable);
  // 消回声开关，默认为true
  void EnableEchoCancellation(bool enable);

private:
  RtcCaptureInternal *m_internal;
  void* m_userdata;
};

#endif
