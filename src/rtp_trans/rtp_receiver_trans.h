#pragma once 

#include "rtp_trans.h"

namespace live_stream_sdk {

  class RTPRecvTrans :public RTPTrans {
  public:
    RTPRecvTrans(StreamId_Ext sid, std::shared_ptr<RTPTransConfig> config);
    ~RTPRecvTrans();

    virtual RTPSession* create_session(RTPTransConfig* config, uint32_t ssrc, RTPAVType payload_type);
  };

}
