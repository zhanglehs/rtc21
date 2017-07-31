#include "../engine_api/rtp_upload.h"

#include "../upload/rtp_upload_internal.h"
#include "../log/log.h"

namespace live_stream_sdk {

  RTPUpload::RTPUpload(const char* deviceid, RtpNetworkObserver* observer) {
    INF("RTPUpload construct %p", this);
    SetLiveSDKDeviceID(deviceid);
    rtp_upload_internal = new RTPUploadInternal(observer);
  }

  RTPUpload::~RTPUpload() {
    INF("~RTPUpload %p", this);
    delete rtp_upload_internal;
  }

  void RTPUpload::DebugLost(uint32_t send_lost_rate, uint32_t recv_lost_rate)
  {
    rtp_upload_internal->SetLostRate(send_lost_rate, recv_lost_rate);
  }

  int32_t RTPUpload::Start(const rtcUploadDispatchConfig* config) {
    return rtp_upload_internal->Start(config);
  }

  void RTPUpload::SetAudioParam(unsigned int frequence, unsigned int frame_size) {
    rtp_upload_internal->SetAudioParam(frequence, frame_size);
  }

  int32_t RTPUpload::SendAAC(char *data, int len, unsigned int timestamp_ms) {
    return rtp_upload_internal->SendAAC(data, len, timestamp_ms);
  }

  int32_t RTPUpload::SendH264(char *data, int len, int width, int height, unsigned int timestamp_ms) {
    return rtp_upload_internal->SendH264(data, len, width, height, timestamp_ms);
  }

  void RTPUpload::GetSSRC(uint32_t &audio, uint32_t &video) {
    rtp_upload_internal->GetSSRC(audio, video);
  }

  RtcCaptureState RTPUpload::GetState() {
    return rtp_upload_internal->GetState();
  }

  RTPTransConfig* RTPUpload::GetRTPTransConfig() {
    return rtp_upload_internal->GetRTPTransConfig();
  }

  void RTPUpload::EnableFec(bool enable) {
    rtp_upload_internal->EnableFec(enable);
  }

  void RTPUpload::EnableNack(bool enable) {
    rtp_upload_internal->EnableNack(enable);
  }

  int32_t RTPUpload::SetNetworkChanged() {
    return rtp_upload_internal->SetNetworkChanged();
  }

  void RTPUpload::SetVideoExpectBitrate(uint32_t bitrate) {
    rtp_upload_internal->SetVideoExpectBitrate(bitrate);
  }

  void RTPUpload::GetVideoEstimate(uint32_t* bitrate, uint8_t* loss, int64_t* rtt) {
    rtp_upload_internal->GetVideoEstimate(bitrate, loss, rtt);
  }

  int32_t RTPUpload::Stop() {
    return rtp_upload_internal->Stop();
  }

  void RTPUpload::GetDispatchConfig(rtcUploadDispatchConfig *config) {
    rtp_upload_internal->GetDispatchConfig(config);
  }

  void RTPUpload::GetNetworkState(UploadNetworkState* state) {
    rtp_upload_internal->GetNetworkState(state);
  }

}
