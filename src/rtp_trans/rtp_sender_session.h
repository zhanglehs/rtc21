#pragma once
/*
* Author: lixingyuan01@youku.com
*/
#include "rtp_session.h"

namespace live_stream_sdk {

  class SendSideBandwidthEstimation;

  class RTPSendSession :public RTPSession {
  public:
    RTPSendSession(RTPTrans * trans, RTPTransConfig* config, uint32_t ssrc, RTPAVType payload_type);
    ~RTPSendSession();
    virtual void on_handle_rtcp(avformat::RTCPPacketInformation *rtcpPacketInformation, uint64_t now);
    virtual void on_timer(uint64_t now);
    void set_expect_bitrate(uint32_t bitrate);
    void set_send_bitrate(uint32_t bitrate);
    bool get_bandwidth_estimate(uint32_t* bitrate, uint8_t* loss, int64_t* rtt);
    bool is_packet_lost_rate_valid() { return _packet_lost_valid; }

  protected:
    virtual void _on_handle_rtp_flexible_fec(const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len, uint64_t now);
    virtual void _on_handle_rtp_fec(const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len, uint64_t now);
    virtual void _on_handle_rtp_nack(const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len, uint64_t now);

  private:
    SendSideBandwidthEstimation *_bandwidth;
    uint32_t _last_rtp_timestamp;
    uint64_t _last_rtp_ntp;
    bool _sending_rtp_packet;
    bool _packet_lost_valid;
    uint64_t _last_send_rtp_ts;
    uint64_t _start_send_rtp_ts;
  };

}
