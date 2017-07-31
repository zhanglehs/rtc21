// edit by zhangle
#include "../engine_api/RtcCapture.h"

#include "../superlogic/RtcCaptureInternal.h"

RtcCapture::RtcCapture(const char* deviceid) {
  m_userdata = 0;
  m_internal = new RtcCaptureInternal(this, deviceid);
}

RtcCapture::~RtcCapture() {
  delete m_internal;
}

int RtcCapture::StartCapture(const lfrtcCaptureConfig* config) {
  return m_internal->StartCapture(config);
}

int RtcCapture::StartPreview(void *window) {
  return m_internal->StartPreview(window);
}

int RtcCapture::StartPreview(T_lfrtcPreviewVideoCb callback, lfrtcRawVideoType videotype) {
  return m_internal->StartPreview(callback, videotype);
}

int RtcCapture::StartEncodeAndSend(const NetworkConfig* net, const lfrtcEncodeConfig* encode, RtcNetworkCallback callback) {
  return m_internal->StartEncodeAndSend(net, encode, callback);
}

void RtcCapture::StopEncodeAndSend() {
  m_internal->StopEncodeAndSend();
}

void RtcCapture::StopPreview() {
  m_internal->StopPreview();
}

void RtcCapture::Stop() {
  m_internal->Stop();
}

RtcCaptureState RtcCapture::GetState(){
  return m_internal->GetState();
}

int RtcCapture::GetAudioRecorderDevice(lfrtcDevice* buf, int count) {
  return RtcCaptureInternal::GetAudioRecorderDevice(buf, count);
}

int RtcCapture::GetCameraDevice(lfrtcDevice* buf, int count) {
  return RtcCaptureInternal::GetCameraDevice(buf, count);
}

int RtcCapture::GetCameraCapability(const char* deviceid, lfrtcCameraCapability* buf, int count) {
  return RtcCaptureInternal::GetCameraCapability(deviceid, buf, count);
}

void RtcCapture::GetAdvancedStateInfo(AdvancedStateInfo *info) {
  m_internal->GetAdvancedStateInfo(info);
}

live_stream_sdk::RTPUpload* RtcCapture::GetUploader() {
  return m_internal->GetUploader();
}

void RtcCapture::EnableNoiseSuppression(bool enable) {
  m_internal->EnableNoiseSuppression(enable);
}

void RtcCapture::EnableEchoCancellation(bool enable) {
  m_internal->EnableEchoCancellation(enable);
}
