#pragma once

#include <stdint.h>

namespace live_stream_sdk {

  class RTPTransConfig {
  public:
    RTPTransConfig();

    void set_default_config();

  public:
    uint32_t max_nack_bandwidth_limit;
    uint32_t nack_interval;
    //receiver
    uint32_t max_nacklst_size; // receiver config, 0: disable nack
    uint32_t max_packet_age;
    uint32_t sr_rr_interval;
    uint32_t remb_interval;
    uint32_t rtt_detect_interval;
    uint32_t session_timeout; //default:3s
    uint32_t min_same_nack_interval;
    uint32_t fec_rtp_count;  // sender config, 0: enable fec, n: one fec protect n packets
    uint32_t fec_interleave_package_val;
    uint32_t fec_l;
    uint32_t fec_d;
    uint32_t fec_d_l;

    int32_t max_buffer_duration_ms;
    uint16_t rtp_port;
    bool enable_suggest_pkt_interval;
    bool enable_inner_jetterbuf;
    bool is_calc_lost_with_retrans;
    int  max_reconnect_seconds;
  };

}
