#include "../rtp_trans/rtp_sender_trans.h"

#include "../avformat/rtcp.h"
#include "../log/log.h"
#include "../rtp_trans/rtp_sender_session.h"
#include "../util/util_common.h"

namespace live_stream_sdk {

  RTPSendTrans::RTPSendTrans(StreamId_Ext sid, std::shared_ptr<RTPTransConfig> config)
    :RTPTrans(sid, SENDER_TRANS_MODE, config) {
    _video_bitrate = 1000000;
    _last_update_packet_lost_rate = 0;
    _frac_packet_lost_rate = 0;
    _rtt_ms = 0;
    _drop_video = false;
    _network_recover_ts = 0;
  }

  RTPSendTrans::~RTPSendTrans() {
  }

  RTPSession* RTPSendTrans::create_session(RTPTransConfig* config, uint32_t ssrc, RTPAVType payload_type) {
    RTPSendSession* session = new RTPSendSession(this, config, ssrc, payload_type);
    session->set_expect_bitrate(_video_bitrate);
    return session;
  }

  void RTPSendTrans::set_video_expect_bitrate(uint32_t bitrate) {
    CLocalLock lock(&this->_mutex);
    for (auto it = _sessions.begin(); it != _sessions.end(); it++) {
      RTPSendSession *session = (RTPSendSession *)it->second;
      if (session) {
        session->set_expect_bitrate(bitrate);
      }
    }
    _video_bitrate = bitrate;
  }

  void RTPSendTrans::get_video_estimate(uint32_t* bitrate, uint8_t* loss, int64_t* rtt) {
    RTPSendSession *session = (RTPSendSession *)get_video_session();
    if (session && session->get_bandwidth_estimate(bitrate, loss, rtt)) {
      *loss = (uint8_t)session->get_frac_packet_lost_rate();
      return;
    }

    session = (RTPSendSession *)get_audio_session();
    if (session && session->get_bandwidth_estimate(bitrate, loss, rtt)) {
      *loss = (uint8_t)session->get_frac_packet_lost_rate();
      if (*bitrate > _video_bitrate / 2) {
        *bitrate = _video_bitrate / 2;
      }
      return;
    }

    *bitrate = 0;
    *loss = 0;
    *rtt = 0;
    TRC("RTPSendTrans::get_video_estimate, no video session");
  }

  void RTPSendTrans::update_packet_lost_rate(uint64_t now) {
    if (_last_update_packet_lost_rate == 0 || now - _last_update_packet_lost_rate > 1000) {
      _last_update_packet_lost_rate = now;

      uint32_t rtt = (uint32_t)-1;
      uint32_t lost = (uint32_t)-1;
      RTPSendSession *session = (RTPSendSession*)get_video_session();
      if (session && !session->is_timeout()) {
        rtt = session->get_rtt_ms();
        if (session->is_packet_lost_rate_valid()) {
          lost = session->get_frac_packet_lost_rate();
        }
      }
      if (rtt == (uint32_t)-1 || lost == (uint32_t)-1) {
        RTPSendSession *session = (RTPSendSession*)get_audio_session();
        if (session && !session->is_timeout()) {
          if (rtt == (uint32_t)-1) {
            rtt = session->get_rtt_ms();
          }
          if (lost == (uint32_t)-1 && session->is_packet_lost_rate_valid()) {
            lost = session->get_frac_packet_lost_rate();
          }
        }
      }
      if (rtt != (uint32_t)-1) {
        _rtt_ms = (_rtt_ms + rtt) / 2;
      }
      if (lost != (uint32_t)-1) {
        _frac_packet_lost_rate = (_frac_packet_lost_rate + lost)/2;
      }

      if (_drop_video) {
        bool drop = _frac_packet_lost_rate * 100 / 255 > 30 || _rtt_ms > 500;
        if (drop) {
          _network_recover_ts = 0;
        }
        else if (_network_recover_ts == 0) {
          _network_recover_ts = _m_clock->TimeInMilliseconds();
        }
        else if (_m_clock->TimeInMilliseconds() - _network_recover_ts > 5000) {
          // delay 5 seconds when network recover.
          _drop_video = false;

          RTPSendSession *audio_session = (RTPSendSession*)get_audio_session();
          RTPSendSession *video_session = (RTPSendSession*)get_video_session();
          uint32_t bitrate = 0;
          uint8_t loss = 0;
          int64_t rtt = 0;
          if (audio_session && video_session
            && audio_session->get_bandwidth_estimate(&bitrate, &loss, &rtt)) {
            if (bitrate > _video_bitrate / 2) {
              bitrate = _video_bitrate / 2;
            }
            video_session->set_send_bitrate(bitrate);
          }
        }
      }
      else {
        _drop_video = _frac_packet_lost_rate * 100 / 255 > 40 || _rtt_ms > 800;
        _network_recover_ts = 0;
      }
    }
  }

}
