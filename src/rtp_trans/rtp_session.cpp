#include "../rtp_trans/rtp_session.h"

#include "../avformat/rtcp.h"
#include "../log/log.h"
#include "../sdk/rtp_format.h"
#include "../media_manager/media_manager_state.h"
#include "../rtp_trans/rtp_config.h"
#include "../rtp_trans/rtp_trans.h"

#ifndef WIN32
#define max std::max
#endif

namespace live_stream_sdk
{
  RTPSession::RTPSession(RTPTrans * trans, RTPTransConfig* config, uint32_t ssrc, RTPAVType payload_type) {
    _parent = trans;
    _payload_type = payload_type;
    _config = config;
    _ssrc = ssrc;
    _m_clock = avformat::Clock::GetRealTimeClock();

    _last_process_sr_ts = 0;

    _total_rtp_packetcount = 0;
    _receiver_bitrate = 0;
    _total_lost_packet_count = 0;
    _frac_packet_lost_rate = 0;
    _lastseq = 0;
    _rtt_ms = 0;

    _closed = false;
    _timeout = false;
    _active_time_ms = _m_clock->TimeInMilliseconds();;
  }

  RTPSession::~RTPSession() {
  }

  int RTPSession::fec_xor_32(void * src, uint32_t src_len, void *dst, uint32_t dst_len) {
    //memset(_pkt_data, 0, 2048);
    memset(_xor_tmp_data, 0, 2048);
    memcpy(_xor_tmp_data, src, src_len);
    //memcpy(_pkt_data, dst, dst_len);
    uint32_t *psrc32 = (uint32_t*)_xor_tmp_data;
    uint32_t *pdst32 = (uint32_t*)dst;
    uint32_t _src_len_32 = (src_len + 7) >> 2;
    uint32_t _dst_len_32 = (dst_len + 7) >> 2;
    for (uint32_t j = 0; j < max(_src_len_32, _dst_len_32); j++) {
      if (j < _src_len_32) {
        *pdst32++ ^= *psrc32++;
      }
      else {
        *pdst32++ ^= 0;
      }
    }
    return 0;
  }

  int RTPSession::fec_xor_64(void * src, uint32_t src_len, void *dst, uint32_t dst_len) {
    //memset(_pkt_data, 0, 2048);
    memset(_xor_tmp_data, 0, 2048);
    memcpy(_xor_tmp_data, src, src_len);
    //memcpy(_pkt_data, dst, dst_len);
    uint64_t *psrc64 = (uint64_t*)_xor_tmp_data;
    uint64_t *pdst64 = (uint64_t*)dst;
    uint32_t _src_len_64 = (src_len + 7) >> 3;
    uint32_t _dst_len_64 = (dst_len + 7) >> 3;
    for (unsigned int j = 0; j < max(_src_len_64, _dst_len_64); j++) {
      if (j < _src_len_64) {
        *pdst64++ ^= *psrc64++;
      }
      else {
        *pdst64++ ^= 0;
      }
    }

    return 0;
  }

  void RTPSession::on_handle_rtcp(avformat::RTCPPacketInformation *rtcpPacketInformation, uint64_t now) {
    if (rtcpPacketInformation->rtcpPacketTypeFlags & avformat::kRtcpBye) {
      _closed = true;
    }

    keepalive(now);
  }

  bool RTPSession::is_newer_seq(uint16_t seq, uint16_t preseq) {
    return (seq != preseq) &&
      static_cast<uint16_t>(seq - preseq) < 0x8000;
  }

  bool RTPSession::isNacklistToolarge() {
    return _config->max_nacklst_size && _nacks.size() > _config->max_nacklst_size;
  }

  bool RTPSession::isNacklistTooOld() {
    return _config->max_packet_age && !_nacks.empty() && uint32_t(_lastseq - _nacks.begin()->first) > _config->max_packet_age;
  }

  uint32_t RTPSession::get_ssrc() {
    return _ssrc;
  }

  uint32_t RTPSession::_send_rtcp(uint32_t ssrc, avformat::RtcpPacket *pkt) {
    return _parent->send_rtcp(ssrc, pkt);
  }

  void RTPSession::on_handle_rtp(const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len, uint64_t now) {
    _on_handle_rtp_fec(pkt, pkt_len, now);

    if (pkt->payload != RTP_AV_FEC && pkt->payload != RTP_AV_FLEXIBLE_FEC) {
      _on_handle_rtp_nack(pkt, pkt_len, now);
    }
  }

  void RTPSession::destroy() {
    avformat::Bye bye;
    bye.From(_ssrc);
    _send_rtcp(_ssrc, &bye);
    DBG("send byte, ssrc %u", _ssrc);
  }

  bool RTPSession::check_timeout(uint64_t now) {
    _timeout =  _closed || now - _active_time_ms > _config->session_timeout;
    return is_timeout();
  }

  bool RTPSession::is_timeout() {
    return _timeout;
  }

  bool RTPSession::is_closed() {
    return _closed;
  }

  void RTPSession::keepalive(uint64_t now) {
    if (!is_closed()) {
      _active_time_ms = now;
    }
  }
}
