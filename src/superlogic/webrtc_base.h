// edit by zhangle
#ifndef _WEBRTC_BASE_H
#define _WEBRTC_BASE_H

#include "../engine_api/RtcCapture.h"

class WebrtcBase {
public:
  WebrtcBase();
  virtual ~WebrtcBase();

  static int GetAudioRecorderDevice(lfrtcDevice* buf, int count);
  static int GetCameraDevice(lfrtcDevice* buf, int count);
  static int GetCameraCapability(const char* deviceid, lfrtcCameraCapability* buf, int count);

private:
  static void EncodedVideoCallback(void *captureCtx, char* data, int len, unsigned int timestamp);
  static void EncodedAudioCallback(void *captureCtx, char* data, int len, unsigned int timestamp);
  static void DecodedVideoCallback(void *playerCtx, char* data[3], lfrtcRawVideoType type, int width, int height);
  static void WebrtcNotifyCallback(void *ctx, unsigned int msgid, int wParam, int lParam);
  static void WebrtcReportCallback(void *ctx, const char *msg);

  virtual void EncodedVideoCallbackImpl(char* data, int len, unsigned int timestamp) {}
  virtual void EncodedAudioCallbackImpl(char* data, int len, unsigned int timestamp) {}
  virtual void DecodedVideoCallbackImpl(char* data[3], lfrtcRawVideoType type, int width, int height) {}
  virtual void WebrtcNotifyCallbackImpl(unsigned int msgid, int wParam, int lParam) {}
  virtual void WebrtcReportCallbackImpl(const char *msg) {}
};

#endif