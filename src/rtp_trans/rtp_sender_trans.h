#pragma once 

#include "rtp_trans.h"

namespace live_stream_sdk {

  class RTPSendTrans : public RTPTrans {
  public:
    RTPSendTrans(StreamId_Ext sid, std::shared_ptr<RTPTransConfig> config);
    virtual ~RTPSendTrans();

    virtual RTPSession* create_session(RTPTransConfig* config, uint32_t ssrc, RTPAVType payload_type);

    void set_video_expect_bitrate(uint32_t bitrate);
    void get_video_estimate(uint32_t* bitrate, uint8_t* loss, int64_t* rtt);

    void update_packet_lost_rate(uint64_t now);
    uint32_t get_frac_packet_lost_rate() { return _frac_packet_lost_rate; }
    uint32_t get_rtt_ms() { return _rtt_ms; }
    bool drop_video_rtp_packet() { return false; } // { return _drop_video; }

  protected:
    uint32_t _video_bitrate;
    uint64_t _last_update_packet_lost_rate;
    uint32_t _frac_packet_lost_rate;
    uint32_t _rtt_ms;
    bool _drop_video;
    uint64_t _network_recover_ts;
  };

}
