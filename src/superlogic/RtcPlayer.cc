#include "../engine_api/RtcPlayer.h"

#include "../superlogic/RtcPlayerInternal.h"

RtcPlayer::RtcPlayer(const char *deviceid) {
  m_userdata = 0;
  m_internal = new RtcPlayerInternal(this, deviceid);
}

RtcPlayer::~RtcPlayer() {
  delete m_internal;
}

int RtcPlayer::Start(const NetworkConfig* net, RtcNetworkCallback callback /*= 0*/) {
  return m_internal->Start(net, callback);
}

int RtcPlayer::Stop() {
  return m_internal->Stop();
}

int RtcPlayer::SetWindow(void *hwnd) {
  return m_internal->SetWindow(hwnd);
}

int RtcPlayer::Snapshot(const char *path) {
  return m_internal->Snapshot(path);
}

void RtcPlayer::Mute(bool mute) {
  m_internal->Mute(mute);
}

void RtcPlayer::SetNetworkChanged() {
  m_internal->SetNetworkChanged();
}

void RtcPlayer::SetDecodedVideoCallback(RtcDecodedVideoCallback callback) {
  m_internal->SetDecodedVideoCallback(callback);
}

RtcPlayerState RtcPlayer::GetState() {
  return m_internal->GetState();
}

live_stream_sdk::RTPDownload * RtcPlayer::GetDownloader() {
  return m_internal->GetDownloader();
}
