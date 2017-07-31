/**
* @file
* @brief
* @author   songshenyi
* <pre><b>copyright: Youku</b></pre>
* <pre><b>email: </b>songshenyi@youku.com</pre>
* <pre><b>company: </b>http://www.youku.com</pre>
* <pre><b>All rights reserved.</b></pre>
* @date 2015/07/24
* @see
*/


#pragma once


#include "media_manager_rtp_interface.h"

#include <map>

namespace media_manager
{

  class RTPMediaManager : MediaManagerRTPInterface
  {
  public:
    RTPMediaManager();

    RTPMediaCache* create_rtp_cache(uint32_t max_duration, uint32_t max_size);
    virtual RTPMediaCache* get_rtp_media_cache(StreamId_Ext& stream_id, int32_t& status_code);
    virtual int32_t init_stream(StreamId_Ext& stream_id);
    virtual void on_timer();

  protected:
    RTPMediaCache* _media_cache;
    StreamId_Ext _stream_id;
  };
}
