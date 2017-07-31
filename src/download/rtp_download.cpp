#include "../engine_api/rtp_download.h"

#include "../download/rtp_download_internal.h"
#include "../log/log.h"

namespace live_stream_sdk {

  RTPDownload::RTPDownload(const char* deviceid, RtpNetworkObserver* observer) {
    INF("RTPDownload construct %p", this);
    SetLiveSDKDeviceID(deviceid);    
    m_internal = new RTPDownloadInternal(observer);
  }

  RTPDownload::~RTPDownload() {
    Stop();
    INF("~RTPDownload %p", this);
    delete m_internal;
    m_internal = NULL;
  }

  int RTPDownload::Start(const rtcDownloadDispatchConfig *config) {
    return m_internal->Start(config);
  }

  RTPTransConfig* RTPDownload::GetRTPTransConfig() {
    return m_internal->GetRTPTransConfig();
  }

  void RTPDownload::EnableFec(bool enable) {
    m_internal->EnableFec(enable);
  }

  void RTPDownload::EnableNack(bool enable) {
    m_internal->EnableNack(enable);
  }

  int32_t RTPDownload::SetNetworkChanged() {
    m_internal->SetNetworkChanged();
    return 0;
  }

  void RTPDownload::DebugLost(uint32_t send_lost_rate, uint32_t recv_lost_rate) {
    m_internal->SetLostRate(send_lost_rate, recv_lost_rate);
  }

  void RTPDownload::GetNetworkState(DownloadNetworkState* state) {
    m_internal->GetNetworkState(state);
  }

  int32_t RTPDownload::Stop() {
    m_internal->Stop();
    return 0;
  }

  void RTPDownload::GetDispatchConfig(rtcDownloadDispatchConfig *config) {
    m_internal->GetDispatchConfig(config);
  }

  RtcPlayerState RTPDownload::GetState() {
    return m_internal->GetState();
  }

}
