// edit by zhangle
#include "../superlogic/webrtc_base.h"

#include "../superlogic/avengine_api.h"
#include "../log/log.h"
#include "../engine_api/rtp_api.h"

WebrtcBase::WebrtcBase() {
  lfrtcInit();
  lfrtcRegisterLogFunc(live_stream_sdk::LogWebrtcPrintf);
  lfrtcSetControlParams(getAvengineParamsSDK());
  lfrtcRegisterAudioEncodeCb(EncodedAudioCallback);
  lfrtcRegisterVideoEncodeCb(EncodedVideoCallback);
  lfrtcRegistReportFunc(WebrtcReportCallback);
  lfrtcRegisterNotifyCb(WebrtcNotifyCallback);
  lfrtcRegisterVideoRawdataCb(DecodedVideoCallback);
}

WebrtcBase::~WebrtcBase() {
}

int WebrtcBase::GetAudioRecorderDevice(lfrtcDevice* buf, int count) {
  lfrtcInit();
  return lfrtcGetAudioRecorderDevice(buf, count);
}

int WebrtcBase::GetCameraDevice(lfrtcDevice* buf, int count) {
  lfrtcInit();
  return lfrtcGetCameraDevice((lfrtcDevice*)buf, count);
}

int WebrtcBase::GetCameraCapability(const char* deviceid, lfrtcCameraCapability* buf, int count) {
  lfrtcInit();
  return lfrtcGetCameraCapability(deviceid, (lfrtcCameraCapability*)buf, count);
}

void WebrtcBase::EncodedVideoCallback(void *captureCtx, char* data, int len, unsigned int timestamp) {
  if (captureCtx) {
    ((WebrtcBase*)captureCtx)->EncodedVideoCallbackImpl(data, len, timestamp);
  }
}

void WebrtcBase::EncodedAudioCallback(void *captureCtx, char* data, int len, unsigned int timestamp) {
  if (captureCtx) {
    ((WebrtcBase*)captureCtx)->EncodedAudioCallbackImpl(data, len, timestamp);
  }
}

void WebrtcBase::DecodedVideoCallback(void *playerCtx, char* data[3], lfrtcRawVideoType type, int width, int height) {
  if (playerCtx) {
    ((WebrtcBase*)playerCtx)->DecodedVideoCallbackImpl(data, type, width, height);
  }
}

void WebrtcBase::WebrtcNotifyCallback(void *ctx, unsigned int msgid, int wParam, int lParam) {
  if (ctx) {
    ((WebrtcBase*)ctx)->WebrtcNotifyCallbackImpl(msgid, wParam, lParam);
  }
}

void WebrtcBase::WebrtcReportCallback(void *ctx, const char *msg) {
  if (ctx) {
    ((WebrtcBase*)ctx)->WebrtcReportCallbackImpl(msg);
  }
}
