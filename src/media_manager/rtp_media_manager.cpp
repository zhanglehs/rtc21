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


#include "rtp_media_manager.h"

using namespace std;

namespace media_manager
{
  RTPMediaManager::RTPMediaManager()
  {
    _media_cache = NULL;
  }

  int32_t RTPMediaManager::init_stream(StreamId_Ext& stream_id)
  {
    _stream_id = stream_id;

    return 0;
  }

  RTPMediaCache* RTPMediaManager::create_rtp_cache(uint32_t max_duration, uint32_t max_size)
  {
    if (_media_cache != NULL)
    {
      delete _media_cache;
    }

    _media_cache = new RTPMediaCache(_stream_id);

    return _media_cache;
  }

  RTPMediaCache* RTPMediaManager::get_rtp_media_cache(StreamId_Ext& stream_id, int32_t& status_code)
  {
    if (stream_id != _stream_id)
    {
      return NULL;
    }

    return _media_cache;
  }


  void RTPMediaManager::on_timer()
  {

  }

}
