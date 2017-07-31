#pragma once
/*
* Author: lixingyuan01@youku.com
*/
#include "rtp_session.h"
#include <set>
#include <deque>

namespace media_manager {
  class RTPBlock;
}

namespace live_stream_sdk {

  class RTPRecvSession :public RTPSession {
  public:
    RTPRecvSession(RTPTrans * trans, RTPTransConfig* config, uint32_t ssrc, RTPAVType payload_type);
    ~RTPRecvSession();
    virtual void on_timer(uint64_t now);

  protected:
    virtual void on_handle_rtcp(avformat::RTCPPacketInformation *rtcpPacketInformation, uint64_t now);
    virtual void _on_handle_rtp_fec(const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len, uint64_t now);
    virtual void _on_handle_rtp_nack(const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len, uint64_t now);
    avformat::RTP_FIXED_HEADER * _recover_with_fec(const avformat::RTP_FIXED_HEADER *pkt, uint16_t pkt_len, uint16_t& out_len, uint64_t now);
    avformat::RTP_FIXED_HEADER * _recover_with_flexible_fec(const avformat::RTP_FIXED_HEADER *pkt, uint16_t pkt_len, uint16_t& out_len, uint64_t now);
    avformat::RTP_FIXED_HEADER * _recover_packet(uint16_t pkt_to_recover, avformat::RTP_FIXED_HEADER * fec_pkt, uint16_t len, uint16_t& out_len);
    avformat::RTP_FIXED_HEADER * _flexible_recover_packet(uint16_t pkt_to_recover, avformat::RTP_FIXED_HEADER * fec_pkt, uint16_t len, uint16_t& out_len);
    int isHisNacklistTooOld(uint16_t seq);

  private:
    typedef std::deque<media_manager::RTPBlock *> PENDING_FEC_QUEUE;
    std::set<uint16_t, SequenceNumberLessThan> _his_nacks;
    PENDING_FEC_QUEUE _pending_fec;
    uint64_t _last_process_rrts;
    uint64_t _last_process_remb;
    uint64_t _last_process_nackts;
    uint64_t _fec_recover_count; //for FEC recover count
    uint64_t _retrans_nack_recover_count;//for NACK recover count
    uint32_t _extend_max_seq;
    uint64_t _last_sr_ntp;
    bool     _pkt_history[65536];
    std::map<uint32_t, uint32_t> _sr_history;
    uint32_t _last_rtp_timestamp;
  };

}
