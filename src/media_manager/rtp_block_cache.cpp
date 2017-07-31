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


#include "rtp_block_cache.h"
#include "cache_watcher.h"
#include "media_manager_rtp_interface.h"
#include "../log/log.h"
#include "time.h"
#include "media_manager_state.h"
#include "../util/util_common.h"
#include "../sdp/sdp.h"
#include <assert.h>

using namespace avformat;
using namespace std;
using namespace live_stream_sdk;

namespace media_manager
{


  RTPCircularCache::RTPCircularCache(StreamId_Ext& stream_id)
  {
    pthread_mutex_init(&_mutex, NULL);
    _media_manager = NULL;
    _stream_id = stream_id;
    _ssrc = 0;
    set_max_size(256 * 16);
  }

  RTPCircularCache::~RTPCircularCache()
  {
    set_max_size(0);
    // TODO: zhangle, destroy mutex
  }

  int32_t RTPCircularCache::set_manager(MediaManagerRTPInterface* media_manager_interface)
  {
    _media_manager = media_manager_interface;
    return 0;
  }

  uint32_t RTPCircularCache::set_max_size(uint32_t max_size)
  {
    _max_size = max_size;

    _adjust();

    return _max_size;
  }

  // if left >= right
  bool seq_ge(uint16_t left, uint16_t right)
  {
    if (left - right < 0x8000)
    {
      return true;
    }
    return false;
  }

  // if left > right
  bool seq_gt(uint16_t left, uint16_t right)
  {
    if ((left != right) && ((uint16_t)(left - right) < 0x8000))
    {
      return true;
    }
    return false;
  }

  // if left < right
  bool seq_lt(uint16_t left, uint16_t right)
  {
    if ((left != right) && ((uint16_t)(right - left) < 0x8000))
    {
      return true;
    }
    return false;
  }

  void RTPCircularCache::_push_back(const avformat::RTP_FIXED_HEADER* rtp, uint16_t len)
  {
    RTPBlock block(rtp, len);
    _circular_cache.push_back(block);

    _lastest_set_seq = rtp->get_seq();
    _set_push_active();
  }

  int32_t RTPCircularCache::empty_count()
  {
    int count = 0;
    for (uint32_t i = 0; i < _circular_cache.size(); i++)
    {
      if (!_circular_cache[i].is_valid())
      {
        count++;
      }
    }

    return count;
  }

  int32_t RTPCircularCache::set_rtp(const RTP_FIXED_HEADER* rtp, uint16_t len, int32_t& status)
  {
    CLocalLock lock(&this->_mutex);
    if (rtp == NULL || len < sizeof(RTP_FIXED_HEADER))
    {
      // invalid data
      status = RTP_CACHE_RTP_INVALID;
      return -1;
    }

    if (_ssrc == 0)
    {
      _ssrc = rtp->get_ssrc();
    }

    if (rtp->get_ssrc() != _ssrc)
    {
      status = RTP_CACHE_RTP_INVALID;
      return -1;
    }

    uint16_t seq = rtp->get_seq();

    if (_circular_cache.size() == 0)
    {
      _push_back(rtp, len);
      status = STATUS_SUCCESS;
      return 0;
    }

    uint16_t latest_len = 0;
    RTP_FIXED_HEADER* latest = _circular_cache.back().get(latest_len);
    if (latest && _sample_rate > 0  && abs(int(latest->get_rtp_timestamp()
      - rtp->get_rtp_timestamp())) >= int(20 * _sample_rate)) {
      _reset();
      _push_back(rtp, len);
      status = STATUS_SUCCESS;
      return 0;
    }

    // this is a circular cache
    // expect_next_max = back.seq + _max_size
    //                         3                      1                    2
    // ||---------|-------------------------|-------------------|------------------||
    //          front.seq                back.seq         expect_next_max
    //            |       cache             |     large seq     |      small seq
    //
    // 1. if back.seq        <  new_block.seq < expect_next_max,  push back and fill empty block
    // 2. if expect_next_max <= new_block.seq < front.seq,         reset cache,
    // 3. if front.seq       <= new_block.seq <= back.seq,         fill in.
    //

    uint16_t back_seq = _circular_cache.back().get_seq();
    uint16_t front_seq = _circular_cache.front().get_seq();
    uint16_t expect_next_max = back_seq + _max_size;

    // 1. if back.seq        < new_block.seq <= expect_next_max,  push back and fill empty block
    if (seq != back_seq && (uint16_t)(seq - back_seq) < _max_size)
    {
      for (uint16_t idx = back_seq + 1; idx != seq; idx++)
      {
        RTPBlock block(NULL, 0);
        _circular_cache.push_back(block);
      }
      _push_back(rtp, len);
      _adjust();

      status = STATUS_SUCCESS;
      return 0;
    }

    // 2. if expect_next_max < new_block.seq < front.seq,         reset cache,
    if ((uint16_t)(seq - expect_next_max) < (uint16_t)(front_seq - expect_next_max))
    {
      _reset();
      _push_back(rtp, len);

      status = STATUS_SUCCESS;
      return 0;
    }

    // 3. if front.seq       <= new_block.seq <= back.seq,         fill in.
    if ((uint16_t)(seq - front_seq) <= (uint16_t)(back_seq - front_seq))
    {
      _fill_in(rtp, len);
      status = STATUS_SUCCESS;
      return 0;
    }

    ERR("unknown error, streamid: %s, rtp seq:%u, rtp len: %u, cache_size: %u",
      _stream_id.unparse().c_str(), rtp->get_seq(), len, _circular_cache.size());
    status = RTP_CACHE_UNKNOWN_ERROR;
    assert(0);
    return -1;
  }

  int RTPCircularCache::_fill_in(const RTP_FIXED_HEADER* rtp, uint16_t len)
  {
    uint16_t seq = rtp->get_seq();
    uint16_t front_seq = _circular_cache.front().get_seq();
    uint32_t idx = (uint16_t)(seq - front_seq);

    if (idx >= _circular_cache.size())
    {
      ERR("unknown error, streamid: %s, rtp seq:%u, front_seq: %u, cache_size: %u",
        _stream_id.unparse().c_str(), rtp->get_seq(), front_seq, _circular_cache.size());
      _reset();
      assert(0);
      return -1;
    }

    RTPBlock& block = _circular_cache[idx];
    if (!block.is_valid())
    {
      block.set(rtp, len);
    }
    else
    {
      TRC("this block is already set. streamid: %s, seq: %u", _stream_id.unparse().c_str(), seq);
    }

    return 0;
  }

  void RTPCircularCache::_set_push_active()
  {
    _push_active = time(NULL);
  }

  uint16_t RTPCircularCache::size()
  {
    return _max_size;
  }

  uint32_t RTPCircularCache::get_ssrc()
  {
    return _ssrc;
  }

  uint32_t RTPCircularCache::_adjust()
  {
    while (_circular_cache.size() > _max_size)
    {
      _circular_cache.front().finalize();
      _circular_cache.pop_front();
    }

    while (_circular_cache.size() > 0 && !_circular_cache.front().is_valid())
    {
      _circular_cache.front().finalize();
      _circular_cache.pop_front();
    }

    return static_cast<uint32_t>(_circular_cache.size());
  }

  void RTPCircularCache::_reset()
  {
    while (_circular_cache.size() > 0)
    {
      RTPBlock& block = _circular_cache.back();
      block.finalize();
      _circular_cache.pop_back();
    }
  }

  RTP_FIXED_HEADER* RTPCircularCache::get_latest(uint16_t& len, int32_t& status_code)
  {
    CLocalLock lock(&this->_mutex);
    if (_circular_cache.size() == 0)
    {
      INF("cache empty");
      status_code = RTP_CACHE_CIRCULAR_CACHE_EMPTY;
      len = 0;
      return NULL;
    }

    RTPBlock& block = _circular_cache.back();

    if (block.is_valid())
    {
      status_code = STATUS_SUCCESS;
      return block.get(len);
    }

    ERR("unknown error, streamid: %s, front_seq: %u, cache_size: %u",
      _stream_id.unparse().c_str(), _circular_cache.front().get_seq(), _circular_cache.size());
    len = 0;
    status_code = RTP_CACHE_UNKNOWN_ERROR;
    assert(0);
    return NULL;
  }

  RTP_FIXED_HEADER* RTPCircularCache::get_by_seq(uint16_t seq, uint16_t& len, int32_t& status_code, bool return_next_valid_packet)
  {
    len = 0;
    CLocalLock lock(&this->_mutex);
    if (_circular_cache.size() == 0)
    {
      INF("cache empty");
      status_code = RTP_CACHE_CIRCULAR_CACHE_EMPTY;
      len = 0;
      return NULL;
    }

    uint16_t back_seq = _circular_cache.back().get_seq();
    uint16_t front_seq = _circular_cache.front().get_seq();

    if (seq_gt(seq, back_seq))
    {
      status_code = RTP_CACHE_SEQ_TOO_LARGE;
      return NULL;
    }

    if (seq_lt(seq, front_seq))
    {
      status_code = RTP_CACHE_SEQ_TOO_SMALL;
      return NULL;
    }

    uint16_t idx = seq - front_seq;
    RTPBlock& block = _circular_cache[idx];

    if (block.is_valid())
    {
      if (seq == block.get_seq())
      {
        status_code = STATUS_SUCCESS;
        return block.get(len);
      }
      else
      {
        ERR("unknown error, streamid: %s, rtp seq:%u, front_seq: %u, cache_size: %u",
          _stream_id.unparse().c_str(), seq, front_seq, _circular_cache.size());
        len = 0;
        status_code = RTP_CACHE_UNKNOWN_ERROR;
        assert(0);
        return NULL;
      }
    }

    if (!return_next_valid_packet)
    {
      status_code = RTP_CACHE_NO_THIS_SEQ;
      len = 0;
      return NULL;
    }
    else
    {
      for (idx = seq - front_seq; idx < _circular_cache.size(); idx++)
      {
        RTPBlock& block = _circular_cache[idx];
        if (block.is_valid())
        {
          status_code = STATUS_SUCCESS;
          return block.get(len);
        }
      }
    }

    ERR("unknown error, streamid: %s, rtp seq:%u, front_seq: %u, cache_size: %u",
      _stream_id.unparse().c_str(), seq, front_seq, _circular_cache.size());
    len = 0;
    status_code = RTP_CACHE_UNKNOWN_ERROR;
    assert(0);
    return NULL;
  }

  RTP_FIXED_HEADER* RTPCircularCache::get_next_by_seq(uint16_t seq, uint16_t& len, int32_t& status_code)
  {
    CLocalLock lock(&this->_mutex);
    seq++;
    return get_by_seq(seq, len, status_code, true);
  }

  uint32_t RTPCircularCache::set_sample_rate(uint32_t sample_rate)
  {
    return _sample_rate = sample_rate;
  }

  uint32_t RTPCircularCache::set_max_duration_ms(uint32_t max_duration)
  {
    return _max_duration = max_duration;
  }


  RTPMediaCache::RTPMediaCache(StreamId_Ext& stream_id)
  {
    _stream_id = stream_id;
    _sdp.reset();
    _media_manager = NULL;

    _audio_cache = NULL;
    _video_cache = NULL;
  }

  RTPMediaCache::~RTPMediaCache()
  {
    if (_audio_cache != NULL)
    {
      delete _audio_cache;
      _audio_cache = NULL;
    }

    if (_video_cache != NULL)
    {
      delete _video_cache;
      _video_cache = NULL;
    }
  }

  int32_t RTPMediaCache::set_manager(MediaManagerRTPInterface* media_manager_interface)
  {
    _media_manager = media_manager_interface;

    return 0;
  }

  int32_t RTPMediaCache::set_sdp(std::string& sdp)
  {
    _sdp.reset(new live_stream_sdk::SdpInfo());
    _sdp->load(sdp.c_str(), static_cast<uint32_t>(sdp.length()));

    if (_media_manager != NULL)
    {
      _media_manager->notify_watcher(_stream_id, CACHE_WATCHING_SDP);
    }

    _reset_cache();

    return 0;
  }

  int32_t RTPMediaCache::set_sdp(const char* sdp, int32_t len)
  {
    _sdp.reset(new SdpInfo());
    _sdp->load(sdp, len);

    if (_media_manager != NULL)
    {
      _media_manager->notify_watcher(_stream_id, CACHE_WATCHING_SDP);
    }

    _reset_cache();

    return 0;
  }

  shared_ptr<live_stream_sdk::SdpInfo> RTPMediaCache::get_sdp()
  {
    return _sdp;
  }

  int32_t RTPMediaCache::_reset_cache()
  {
    if (_audio_cache != NULL)
    {
      delete _audio_cache;
      _audio_cache = NULL;
    }

    if (_video_cache != NULL)
    {
      delete _video_cache;
      _video_cache = NULL;
    }

    const vector<rtp_media_info*>& _media_infos = _sdp->get_media_infos();

    for (uint32_t i = 0; i < _media_infos.size(); i++)
    {
      RTPMediaType media_type;
      rtp_media_info* media_info = _media_infos[i];
      switch (media_info->payload_type)
      {
      case RTP_AV_H264:
        media_type = RTP_MEDIA_VIDEO;
        break;
      case RTP_AV_AAC:
      case RTP_AV_MP3:
        media_type = RTP_MEDIA_AUDIO;
        break;
      default:
        media_type = RTP_MEDIA_NULL;
        break;
      }

      switch (media_type)
      {
      case avformat::RTP_MEDIA_NULL:
        break;
      case avformat::RTP_MEDIA_AUDIO:
        if (_audio_cache == NULL)
        {
          _audio_cache = new RTPCircularCache(_stream_id);
          _audio_cache->set_sample_rate(media_info->rate);
          _audio_cache->set_max_size(1000);
          _audio_cache->set_max_duration_ms(10000);
        }
        break;
      case avformat::RTP_MEDIA_VIDEO:
        if (_video_cache == NULL)
        {
          _video_cache = new RTPCircularCache(_stream_id);
          _video_cache->set_sample_rate(media_info->rate);
          _video_cache->set_max_size(3000);
          _video_cache->set_max_duration_ms(10000);
        }
        break;
      case avformat::RTP_MEDIA_BOTH:
        break;
      default:
        break;
      }
    }

    return 0;
  }
  //get cache: audio or video
  RTPCircularCache* RTPMediaCache::get_cache_by_ssrc(uint32_t ssrc)
  {
    if (_audio_cache != NULL)
    {
      if (_audio_cache->get_ssrc() == ssrc)
      {
        return _audio_cache;
      }
    }

    if (_video_cache != NULL)
    {
      if (_video_cache->get_ssrc() == ssrc)
      {
        return _video_cache;
      }
    }

    return NULL;
  }

  RTPCircularCache* RTPMediaCache::get_cache_by_media(avformat::RTPMediaType media_type)
  {
    switch (media_type)
    {
    case avformat::RTP_MEDIA_NULL:
      break;
    case avformat::RTP_MEDIA_AUDIO:
      return _audio_cache;
      break;
    case avformat::RTP_MEDIA_VIDEO:
      return _video_cache;
      break;
    case avformat::RTP_MEDIA_BOTH:
      break;
    default:
      break;
    }

    return NULL;
  }

  //RTPCircularCache* RTPMediaCache::get_cache_by_codec(RTPAVType av_type)
  //{
  //  return NULL;
  //}

  RTPCircularCache* RTPMediaCache::get_audio_cache()
  {
    return _audio_cache;
  }

  RTPCircularCache* RTPMediaCache::get_video_cache()
  {
    return _video_cache;
  }

  void RTPMediaCache::adjust_cache_size()
  {

  }

  int32_t RTPMediaCache::set_rtp(const RTP_FIXED_HEADER* rtp, uint16_t len, int32_t& status)
  {
    if (rtp == NULL || len < sizeof(RTP_FIXED_HEADER))
    {
      status = RTP_CACHE_RTP_INVALID;
      return status;
    }

    RTPAVType type = (RTPAVType)rtp->payload;
    switch (type)
    {
    case RTP_AV_MP3:
    case RTP_AV_AAC:
      if (_audio_cache == NULL)
      {
        ERR("_audio_cache is NULL");
        status = RTP_CACHE_SET_RTP_FAILED;
        return status;
      }

      _audio_cache->set_rtp(rtp, len, status);
      if (status == STATUS_SUCCESS && _media_manager != NULL)
      {
        _media_manager->notify_watcher(_stream_id, CACHE_WATCHING_RTP_BLOCK);
      }
      break;
    case RTP_AV_H264:
      if (_video_cache == NULL)
      {
        ERR("_video_cache is NULL");
        status = RTP_CACHE_SET_RTP_FAILED;
        return status;
      }

      _video_cache->set_rtp(rtp, len, status);
      if (status == STATUS_SUCCESS && _media_manager != NULL)
      {
        _media_manager->notify_watcher(_stream_id, CACHE_WATCHING_RTP_BLOCK);
      }
      break;
    case RTP_AV_ALL:
    case RTP_AV_NULL:
    default:
      // ERROR
      status = RTP_CACHE_SET_RTP_FAILED;
    }
    return status;
  }

}
