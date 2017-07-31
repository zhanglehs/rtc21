#include "../rtp_trans/rtp_receiver_trans.h"

#include "../rtp_trans/rtp_receiver_session.h"

namespace live_stream_sdk {

  RTPRecvTrans::RTPRecvTrans(StreamId_Ext sid, std::shared_ptr<RTPTransConfig> config)
    :RTPTrans(sid, RECEIVER_TRANS_MODE, config) {
  }

  RTPRecvTrans::~RTPRecvTrans() {
  }

  RTPSession* RTPRecvTrans::create_session(RTPTransConfig* config, uint32_t ssrc, RTPAVType payload_type) {
    return new RTPRecvSession(this, config, ssrc, payload_type);
  }

}
