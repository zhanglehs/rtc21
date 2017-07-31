/*
* Author: gaosiyu@youku.com, hechao@youku.com
*/
#include "../rtp_trans/rtp_trans.h"

#include "../avformat/rtcp.h"
#include "../cmd_protocol/proto_rtp_rtcp.h"
#include "../log/log.h"
#include "../media_manager/rtp_block_cache.h"
#include "../rtp_trans/rtp_config.h"
#include "../rtp_trans/rtp_sender_session.h"
#include "../rtp_trans/rtp_sender_trans.h"
#include "../rtp_trans/rtp_receiver_session.h"
#include "../util/data_buffer.h"
#include "../util/util_common.h"
#include "../sdk/rtp_format.h"

using namespace avformat;
using namespace media_manager;

namespace live_stream_sdk {

  RTPTrans::RTPTrans(StreamId_Ext sid, RTPTransMode mode, std::shared_ptr<RTPTransConfig> config)
    : _sid(sid), _mode(mode), _config(config) {
    pthread_mutex_init(&_mutex, NULL);
    _m_clock = avformat::Clock::GetRealTimeClock();
    _rtcpParser = new avformat::RTCP(_m_clock);
    _init_ts = _m_clock->TimeInMilliseconds();
    _is_alive = true;

    _buffer = new live_stream_sdk::DataBuffer(256 * 256);
    _media_cache = NULL;
    _audio_time_base = 48000;
    _video_time_base = 90000;
  }

  bool RTPTrans::is_alive() {
    return _is_alive;
  }

  RTPSession *RTPTrans::get_video_session() {
    for (std::map<uint32_t, RTPSession *>::iterator it = _sessions.begin();
      it != _sessions.end(); it++) {
      RTPSession *session = it->second;
      if (session->get_payload_type() == live_stream_sdk::RTP_AV_H264) {
        return session;
      }
    }
    return NULL;
  }

  RTPSession *RTPTrans::get_audio_session() {
    for (std::map<uint32_t, RTPSession *>::iterator it = _sessions.begin();
      it != _sessions.end(); it++) {
      RTPSession *session = it->second;
      if (session->get_payload_type() == live_stream_sdk::RTP_AV_AAC) {
        return session;
      }
    }
    return NULL;
  }

  uint32_t RTPTrans::get_video_ssrc() {
    CLocalLock lock(&_mutex);
    RTPSession *session = get_video_session();
    if (session) {
      return session->get_ssrc();
    }
    return 0;
  }

  uint32_t RTPTrans::get_audio_ssrc() {
    CLocalLock lock(&_mutex);
    RTPSession *session = get_audio_session();
    if (session) {
      return session->get_ssrc();
    }
    return 0;
  }

#ifdef FUNC_IMPL
#undef FUNC_IMPL
#endif
#define FUNC_IMPL(ret, name) \
ret RTPTrans::get_video_##name() { \
  CLocalLock lock(&_mutex); \
  RTPSession *session = get_video_session(); \
  if (session && !session->is_timeout()) { \
    return session->get_##name(); \
  } \
  return 0; \
} \
\
ret RTPTrans::get_audio_##name() { \
  CLocalLock lock(&_mutex); \
  RTPSession *session = get_audio_session(); \
  if (session && !session->is_timeout()) { \
    return session->get_##name(); \
  } \
  return 0; \
}

  FUNC_IMPL(uint32_t, frac_packet_lost_rate)
  FUNC_IMPL(uint32_t, total_lost_packet_count)
  FUNC_IMPL(uint32_t, total_rtp_packetcount)
  FUNC_IMPL(uint32_t, rtt_ms)
  FUNC_IMPL(uint32_t, current_bitrate)
  FUNC_IMPL(uint32_t, effect_bitrate)
  FUNC_IMPL(uint64_t, total_rtp_bytes)

  RTPTrans::~RTPTrans() {
    TRC("~RTPTrans");
    CLocalLock lock(&this->_mutex);
    clear_sessions();
    delete _rtcpParser;
    delete _buffer;
  }

  void RTPTrans::reset(StreamId_Ext id) {
    _sid = id;
    _is_alive = true;
    if (_channel.get()) {
      _channel->close();
    }
      
    CLocalLock lock(&this->_mutex);
    clear_sessions();
    _init_ts = _m_clock->TimeInMilliseconds();
  }

  int32_t RTPTrans::set_channel(std::shared_ptr<network::BaseNetworkChannel> channel) {
    if (channel == NULL) {
      ERR("channel is NULL");
      return -1;
    }
    _channel = channel;
    return 0;
  }

  int RTPTrans::recv_rtp(uint32_t ssrc, const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len) {
    if (_fec_rtp_callback) {
      _fec_rtp_callback((uint8_t *)pkt, pkt_len, _fecdata);
    }

    return 0;
  }

  int RTPTrans::send_rtp(uint32_t ssrc, const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len) {
    int ret = 0;
    try {
      if (_channel != NULL) {
        _buffer->clear();
        StreamId_Ext id = get_sid();
        encode_rtp_u2r_packet(id, (uint8_t*)pkt, pkt_len, _buffer);
        int send_bytes = _channel->send_data((char*)_buffer->data_ptr(), static_cast<uint32_t>(_buffer->data_len()));
        ret += send_bytes;
      }
    }
    catch (BaseException& ex) {
      ERR("%s", ex.GetNameAndError().c_str());
    }

    return ret;
  }

  int RTPTrans::send_rtcp(uint32_t ssrc, const avformat::RtcpPacket *pkt) {
    try {
      const size_t max_len = 2048;
      size_t len = 0;
      uint8_t rtcp[max_len];
      pkt->Build(rtcp, &len, max_len);
      if (len == 0) {
        return -1;
      }

      if (_channel != NULL) {
        _buffer->clear();
        StreamId_Ext id = get_sid();
        if (_mode == SENDER_TRANS_MODE) {
          encode_rtcp_u2r_packet(id, rtcp, len, _buffer);
        }
        else {
          encode_rtcp_d2p_packet(id, rtcp, len, _buffer);
        }
        _channel->send_data((char*)_buffer->data_ptr(), static_cast<uint32_t>(_buffer->data_len()));
      }
    }
    catch (BaseException& ex) {
      if (ex.GetNameAndError() != _last_error) {
        _last_error = ex.GetNameAndError();
        ERR("%s", ex.GetNameAndError().c_str());
      }
    }

    return 0;
  }

  avformat::RTP_FIXED_HEADER* RTPTrans::get_rtp_by_ssrc_seq(uint32_t ssrc, uint16_t seq, uint16_t &len, int32_t& status_code) {
#ifndef RTP_SERVER
    RTPCircularCache* cache = _media_cache->get_cache_by_ssrc(ssrc);//get cache: audio or video
    if (cache == NULL) {
      WRN("can not find circular cache, ssrc: %u", ssrc);
      return NULL;
    }

    int32_t status = 0;
    return cache->get_by_seq(seq, len, status, false);
#else
    return NULL;
#endif
  }

  void RTPTrans::clear_sessions() {
    // should lock before call this function
    for (std::map<uint32_t, RTPSession *>::iterator it = _sessions.begin();
      it != _sessions.end(); it++) {
      RTPSession *session = it->second;
      delete session;
    }
    _sessions.clear();
  }

  void RTPTrans::on_handle_rtp(const uint8_t* data, uint32_t pkt_len) {
    CLocalLock lock(&this->_mutex);
    if (data == NULL || pkt_len <= 0 || !_is_alive) {
      return;
    }

    const RTP_FIXED_HEADER* pkt = (const RTP_FIXED_HEADER*)data;
    uint32_t ssrc = pkt->get_ssrc();
    if (ssrc <= 0) {
      return;
    }

    RTPSession* session = NULL;
    auto it = _sessions.find(ssrc);
    if (it == _sessions.end()) {
      session = create_session(_config.get(), ssrc, RTPAVType(pkt->payload));
      _sessions[ssrc] = session;
    }
    else {
      session = it->second;
    }

    if (!session->is_closed()) {
      uint64_t now = _m_clock->TimeInMilliseconds();
      session->on_handle_rtp(pkt, pkt_len, now);
    }
  }

  bool RTPTrans::on_handle_rtcp(const uint8_t *data, uint32_t data_len, int *payloadtype) {
    *payloadtype = 0;

    CLocalLock lock(&this->_mutex);
    if (data == NULL || data_len <= 0 || !_is_alive) {
      return false;
    }

    avformat::RTCPPacketInformation rtcpInfo;
    _rtcpParser->parse_rtcp_packet(data, data_len, rtcpInfo);
    uint32_t ssrc = rtcpInfo.remoteSSRC;
    bool isSR = (rtcpInfo.rtcpPacketTypeFlags & avformat::kRtcpSr) != 0;
    if (ssrc <= 0) {
      return false;
    }

    RTPSession* session = NULL;
    auto it = _sessions.find(ssrc);
    if (it == _sessions.end()) {
      return false;
    }
    else {
      session = it->second;
    }

    if (!session->is_closed()) {
      uint64_t now = _m_clock->TimeInMilliseconds();
      session->on_handle_rtcp(&rtcpInfo, now);
      *payloadtype = session->get_payload_type();
    }

    return isSR;
  }

  void RTPTrans::on_timer() {
    CLocalLock lock(&_mutex);

    uint64_t now = _m_clock->TimeInMilliseconds();
    if (_sessions.empty()) {
      _is_alive = now - _init_ts < _config->session_timeout;
    }
    else {
      bool is_alive = false;
      for (std::map<uint32_t, RTPSession *>::iterator it = _sessions.begin(); it != _sessions.end(); it++) {
        RTPSession *session = it->second;
        session->on_timer(now);
        is_alive = is_alive || (!session->check_timeout(now));
      }
      _is_alive = is_alive;
    }

    if (!_is_alive)
    {
      _config->session_timeout++;
      if (_config->session_timeout > RTP_SESSION_MAX_TIMEOUT)
      {
        _config->session_timeout = RTP_SESSION_MAX_TIMEOUT;
      }
    }

    if (!is_alive()) {
      clear_sessions();
      INF("close_session sid: %s, _is_alive: %d, ", _sid.unparse().c_str(), _is_alive);
    }

    if (_mode == SENDER_TRANS_MODE) {
      ((RTPSendTrans*)this)->update_packet_lost_rate(now);
    }
  }

  void RTPTrans::destroy() {
    TRC("destroy Lock");
    CLocalLock lock(&this->_mutex);
    for (std::map<uint32_t, RTPSession *>::iterator it = _sessions.begin(); it != _sessions.end(); it++) {
      RTPSession *session = it->second;
      session->destroy();
      delete session;
    }
    _sessions.clear();
    INF("rtp trans destroyed");
  }

}
