#include "../sdk/rtp_format.h"

#include "../rtp_trans/rtp_config.h"
#include "../log/log.h"
#include <string.h>

namespace live_stream_sdk {

  RTPTransConfig::RTPTransConfig() {
    set_default_config();
  }

  void RTPTransConfig::set_default_config() {
    max_nack_bandwidth_limit = MAX_NACK_BITRATE;
    INF("default max_nack_bandwidth_limit=%u", max_nack_bandwidth_limit);
    nack_interval = NACK_PROCESS_INERVAL;
    max_nacklst_size = NACK_LIST_MAX_SIZE;
    max_packet_age = NACK_PKT_MAX_AGE;
    sr_rr_interval = SR_RR_INERVAL;
    remb_interval = REMB_INTERVAL;
    rtt_detect_interval = RTT_INERVAL;
    session_timeout = RTP_SESSION_TIMEOUT;
    INF("default session_timeout=%d", session_timeout);
    enable_suggest_pkt_interval = ENABLE_SUGGEST_PKT_INTERVAL;
    enable_inner_jetterbuf = ENABLE_INNER_JETTERBUF;
    max_reconnect_seconds = MAX_RECONNECT_SECONDS;
    min_same_nack_interval = MIN_SAME_NACK_INTERVAL;
    is_calc_lost_with_retrans = IS_CALC_LOST_WITH_RETRANS;
    fec_rtp_count = 0;// FEC_RTP_COUNT;
    INF("default fec_rtp_count=%d", fec_rtp_count);
    fec_interleave_package_val = FEC_INTERLEAVE_PACKAGE_VAL;
    INF("default fec_interleave_package_val=%d", fec_interleave_package_val);
    fec_l = 9;
    fec_d = 5;
    fec_d_l = 9;
  }

}
