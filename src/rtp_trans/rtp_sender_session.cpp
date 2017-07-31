/*
* Author: lixingyuan01@youku.com
*/
#include "../rtp_trans/rtp_sender_session.h"

#include "../avformat/rtcp.h"
#include "../log/log.h"
#include "../sdk/rtp_format.h"
#include "../media_manager/media_manager_state.h"
#include "../rtp_trans/rtp_config.h"
#include "../rtp_trans/rtp_trans.h"
#include "../rtp_trans/rtp_sender_trans.h"
#include "../rtp_trans/send_side_bandwidth_estimation.h"
#include <algorithm>

#ifndef WIN32
#define min std::min
#endif

namespace live_stream_sdk {

  RTPSendSession::RTPSendSession(RTPTrans * trans, RTPTransConfig* config, uint32_t ssrc, RTPAVType payload_type)
    :RTPSession(trans, config, ssrc, payload_type) {
    _bandwidth = new SendSideBandwidthEstimation();
    _last_rtp_timestamp = 0;
    _last_rtp_ntp = 0;
    _sending_rtp_packet = false;
    _packet_lost_valid = false;
    _last_send_rtp_ts = 0;
    _start_send_rtp_ts = 0;
  }

  RTPSendSession::~RTPSendSession() {
    delete _bandwidth;
  }

  void RTPSendSession::on_timer(uint64_t now) {
    if (_last_process_sr_ts == 0 || now - _last_process_sr_ts > _config->sr_rr_interval) {
      if (_last_rtp_ntp > 0) {
        uint32_t freq = 0;
        if (_payload_type == RTP_AV_AAC) {
          freq = 48;
        }
        else {
          freq = 90;
        }
        avformat::SenderReport sr;
        uint32_t ntp_secs, ntp_frac;
        _m_clock->CurrentNtp(ntp_secs, ntp_frac);//NTP Timestamp

        sr.From(_ssrc);
        sr.WithNtpSec(ntp_secs);
        sr.WithNtpFrac(ntp_frac);
        //diff to RFC3550,we use this field to transfer rtt to receiver
        sr.WithRtpTimestamp(_last_rtp_timestamp + uint32_t(now - _last_rtp_ntp) * freq);
        sr.WithOctetCount((uint32_t)_speed.get_total_bytes());
        sr.WithPacketCount(_total_rtp_packetcount);

        avformat::Sdes seds;
        seds.WithCName(_ssrc, RTP_CNAME);
        sr.Append(&seds);

        _send_rtcp(_ssrc, &sr);
        _last_process_sr_ts = now;
      }
    }

    _sending_rtp_packet = now - _last_send_rtp_ts < 3000;
    _packet_lost_valid = _sending_rtp_packet && now - _start_send_rtp_ts > 3000;

    _speed.update_speed(now);
  }

  void RTPSendSession::_on_handle_rtp_flexible_fec(const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len, uint64_t now)
  {
    if (_config->fec_l > 1 && _config->fec_d_l > 1 && _config->fec_d > 0 && pkt->get_seq() > 0) {
      //generate fec pkt
      int r_fec = 1;
      int c_fec = 1;
      r_fec = pkt->get_seq() % _config->fec_l;
      c_fec = (pkt->get_seq() - 1) % _config->fec_d;

      if (!r_fec || !c_fec)
      {
        memset(_pkt_data, 0, 2048);
        avformat::RTP_FIXED_HEADER *rtp_pkt = (avformat::RTP_FIXED_HEADER *)_pkt_data;
        avformat::FLEX_FEC_HEADER *fec_pkt = (avformat::FLEX_FEC_HEADER *)(rtp_pkt->data);

        uint16_t rtp_pkt_len = 0;
        uint8_t *fec_data = fec_pkt->data;
        rtp_pkt->version = 2;
        rtp_pkt->marker = 1;
        rtp_pkt->csrc_len = 0;
        rtp_pkt->extension = 0;
        rtp_pkt->padding = 0;
        rtp_pkt->payload = RTP_AV_FLEXIBLE_FEC;

        uint8_t payload = 0;
        uint32_t timestamp = 0;
        uint16_t length = 0;
        uint8_t is_first = 1;
        int32_t status_code = 0;
        uint8_t mark = 0;
        uint16_t seq_base = r_fec == 0 ? pkt->get_seq() - (_config->fec_l - 1) : pkt->get_seq() - _config->fec_d * (_config->fec_d_l - 1);

        uint16_t fec_interleave = r_fec == 0 ? 1 : _config->fec_d;
        uint16_t fec_count = r_fec == 0 ? _config->fec_l : _config->fec_d_l;

        char seq_rtcp[1024];
        char seq_temp[128];
        seq_rtcp[0] = '\0';

        for (int i = 0, index = 0; i < fec_count; i++, index += fec_interleave)
        {
          uint16_t t_len = 0;
          avformat::RTP_FIXED_HEADER *t_pkt = _parent->get_rtp_by_ssrc_seq(_ssrc, static_cast<uint32_t>(seq_base + index), t_len, status_code);

          if (media_manager::STATUS_SUCCESS == status_code && t_pkt)
          {
            sprintf(seq_temp, "%d,", (uint16_t)(seq_base + index));
            if (strlen(seq_rtcp) < 1000)
            {
              strcat(seq_rtcp, seq_temp);
            }
            if (is_first)
            {
              payload = t_pkt->payload;
              timestamp = t_pkt->get_rtp_timestamp();
              length = t_len;
              rtp_pkt->set_rtp_timestamp(t_pkt->get_rtp_timestamp());
              rtp_pkt->set_ssrc(_ssrc);
              memcpy(fec_data, t_pkt->data, t_len - sizeof(avformat::RTP_FIXED_HEADER));
              is_first = 0;
              rtp_pkt_len = t_len;
              mark = t_pkt->marker;
              //fec_pkt->data[0] = *((uint8_t*)t_pkt);
            }
            else {
              payload ^= t_pkt->payload;
              timestamp ^= t_pkt->get_rtp_timestamp();
              length ^= t_len;
              //fec_pkt->data[0] ^= *((uint8_t*)t_pkt);
              mark ^= t_pkt->marker;
              if (rtp_pkt_len < t_len)
              {
                rtp_pkt_len = t_len;
              }
#ifdef WIN32
              fec_xor_64(t_pkt->data, t_len - sizeof(avformat::RTP_FIXED_HEADER), fec_data, 2048 - sizeof(avformat::RTP_FIXED_HEADER));
#else
              fec_xor_32(t_pkt->data, t_len - sizeof(avformat::RTP_FIXED_HEADER), fec_data, 2048 - sizeof(avformat::RTP_FIXED_HEADER));
#endif
            }
          }
        }
        fec_pkt->marker = mark;
        rtp_pkt->set_seq(static_cast<uint16_t>(seq_base + fec_interleave*(fec_count - 1)));
        fec_pkt->set_sn_base(seq_base);
        fec_pkt->set_length_recovery(length);
        fec_pkt->set_payload_type(payload);
        fec_pkt->set_timestamp(timestamp);
        if (!r_fec)
        {
          fec_pkt->set_M(_config->fec_l);
          fec_pkt->set_N(0);
        }
        else
        {
          fec_pkt->set_M(_config->fec_d_l);
          fec_pkt->set_N(_config->fec_d);
        }
        fec_pkt->set_MSK(3);
        //int send_bytes = _parent->send_rtp(_ssrc, rtp_pkt, rtp_pkt_len + sizeof(avformat::FLEX_FEC_HEADER));
        _speed.add_bytes(rtp_pkt_len + sizeof(avformat::FLEX_FEC_HEADER));

        TRC_NET("RTCP_S_FEC streamid %s ssrc %u seq %d payload %d seqnumber %s", _parent->_sid.unparse().c_str(), _ssrc, rtp_pkt->get_seq(), rtp_pkt->payload, seq_rtcp);
        TRC_NET("RTP_S_FEC streamid %s ssrc %u seq %d payload %d timestamp %u len %d", _parent->_sid.unparse().c_str(), rtp_pkt->get_ssrc(), rtp_pkt->get_seq(), rtp_pkt->payload, rtp_pkt->get_rtp_timestamp(), rtp_pkt_len + sizeof(avformat::FLEX_FEC_HEADER));
      }
    }
  }

  void RTPSendSession::_on_handle_rtp_fec(const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len, uint64_t now)
  {
    uint32_t fec_rtp_count = _config->fec_rtp_count;
    if (_payload_type == RTP_AV_AAC) {
      fec_rtp_count = FEC_RTP_COUNT;
    }
    if (fec_rtp_count == 0) {
      return;
    }

    if (_config->fec_interleave_package_val > 0) {
      //generate fec pkt
      if (pkt->get_seq() > 0
        && (pkt->get_seq() % (_config->fec_interleave_package_val* fec_rtp_count)) == 0)
      {
        for (unsigned int j = 0; j < _config->fec_interleave_package_val; j++)
        {
          memset(_pkt_data, 0, 2048);
          avformat::RTP_FIXED_HEADER *rtp_pkt =
            (avformat::RTP_FIXED_HEADER *) _pkt_data;
          avformat::FEC_HEADER *fec_pkt =
            (avformat::FEC_HEADER *) (rtp_pkt->data);
          avformat::FEC_EXTEND_HEADER *fec_ext =
            (avformat::FEC_EXTEND_HEADER *) fec_pkt->data;
          uint16_t rtp_pkt_len = 0;
          uint8_t *fec_data = fec_pkt->data
            + sizeof(avformat::FEC_EXTEND_HEADER);
          rtp_pkt->version = 2;
          rtp_pkt->marker = 1;
          rtp_pkt->csrc_len = 0;
          rtp_pkt->extension = 0;
          rtp_pkt->padding = 0;
          rtp_pkt->payload = RTP_AV_FEC;

          uint8_t payload = 0;
          uint32_t timestamp = 0;
          uint16_t length = 0;
          uint8_t is_first = 1;
          int32_t status_code = 0;
          uint32_t mask = 0;
          uint8_t mark = 0;
          uint16_t seq_base = pkt->get_seq() - fec_rtp_count  * _config->fec_interleave_package_val + 1 + j;

          char seq_rtcp[1024];
          char seq_temp[128];
          seq_rtcp[0] = '\0';
          for (unsigned int i = 0; i < fec_rtp_count; i++) {
            uint16_t t_len = 0;
            avformat::RTP_FIXED_HEADER *t_pkt =
              _parent->get_rtp_by_ssrc_seq(_ssrc,
              static_cast<uint32_t>(seq_base + i *_config->fec_interleave_package_val), t_len,
              status_code);

            if (media_manager::STATUS_SUCCESS == status_code && t_pkt) {

              sprintf(seq_temp, "%d,", (uint16_t)(seq_base + i *_config->fec_interleave_package_val));
              if (strlen(seq_rtcp) < 1000)
              {
                strcat(seq_rtcp, seq_temp);
              }

              if (is_first) {
                payload = t_pkt->payload;
                timestamp = t_pkt->get_rtp_timestamp();
                length = t_len;
                rtp_pkt->set_rtp_timestamp(t_pkt->get_rtp_timestamp());
                rtp_pkt->set_ssrc(_ssrc);
                memcpy(fec_data, t_pkt->data,
                  t_len - sizeof(avformat::RTP_FIXED_HEADER));
                is_first = 0;
                rtp_pkt_len = t_len;
                mark = t_pkt->marker;
                fec_pkt->data[0] = *((uint8_t*)t_pkt);
              }
              else {
                payload ^= t_pkt->payload;
                timestamp ^= t_pkt->get_rtp_timestamp();
                length ^= t_len;
                fec_pkt->data[0] ^= *((uint8_t*)t_pkt);
                mark ^= t_pkt->marker;
                if (rtp_pkt_len < t_len) {
                  rtp_pkt_len = t_len;
                }
#ifdef WIN32
                fec_xor_64(t_pkt->data, t_len - sizeof(avformat::RTP_FIXED_HEADER), fec_data, 2048 - sizeof(avformat::RTP_FIXED_HEADER));
#else
                fec_xor_32(t_pkt->data, t_len - sizeof(avformat::RTP_FIXED_HEADER), fec_data, 2048 - sizeof(avformat::RTP_FIXED_HEADER));
#endif

              }
              mask |= (1 << (i *_config->fec_interleave_package_val));
            }
          }
          if (!is_first)
          {
            fec_ext->marker = mark;
            rtp_pkt->set_seq(static_cast<uint16_t>(seq_base + fec_rtp_count - 1));
            fec_pkt->set_sn_base(seq_base);
            fec_pkt->set_length_recovery(length);
            fec_pkt->set_payload_type(payload);
            fec_pkt->set_timestamp(timestamp);
            fec_pkt->set_mask(mask);
            fec_pkt->set_extension(1);

            //int send_bytes = _parent->send_rtp(_ssrc, rtp_pkt,
            //  rtp_pkt_len + sizeof(avformat::FEC_HEADER)
            //  + sizeof(avformat::FEC_EXTEND_HEADER));
            _speed.add_bytes(rtp_pkt_len + sizeof(avformat::FEC_HEADER) + sizeof(avformat::FEC_EXTEND_HEADER));

            TRC_NET("RTCP_S_FEC streamid %s ssrc %u seq %d payload %d seqnumber %s", _parent->_sid.unparse().c_str(), _ssrc, rtp_pkt->get_seq(), rtp_pkt->payload, seq_rtcp);
            TRC_NET("RTP_S_FEC streamid %s ssrc %u seq %d payload %d timestamp %u len %d", _parent->_sid.unparse().c_str(), rtp_pkt->get_ssrc(), rtp_pkt->get_seq(), rtp_pkt->payload, rtp_pkt->get_rtp_timestamp(), rtp_pkt_len + sizeof(avformat::FEC_HEADER) + sizeof(avformat::FEC_EXTEND_HEADER));
          }
        }
      }
    }
    else {
      _on_handle_rtp_flexible_fec(pkt, pkt_len, now);
    }
  }

  void RTPSendSession::_on_handle_rtp_nack(const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len, uint64_t now) {
    uint16_t seq = pkt->get_seq();
    if (is_newer_seq(seq, _lastseq)) {
      _lastseq = seq;
    }

    _last_rtp_timestamp = pkt->get_rtp_timestamp();
    _last_rtp_ntp = now;
    _speed.add_bytes(pkt_len);
    _total_rtp_packetcount++;

    while (isNacklistToolarge() || isNacklistTooOld()) {
      _nacks.erase(_nacks.begin());
    }

    _last_send_rtp_ts = now;
    if (!_sending_rtp_packet) {
      _start_send_rtp_ts = now;
    }
  }

  void RTPSendSession::on_handle_rtcp(avformat::RTCPPacketInformation *rtcpPacketInformation, uint64_t now) {
    if (rtcpPacketInformation->rtcpPacketTypeFlags & avformat::kRtcpNack) {
      std::vector<uint16_t> nack_items = rtcpPacketInformation->nackList;

      char seq_rtcp[1024];
      char seq_temp[128];
      seq_rtcp[0] = '\0';
      for (std::vector<uint16_t>::iterator it = nack_items.begin(); it != nack_items.end(); it++) {
        uint16_t seq = *it;
        sprintf(seq_temp, "%d,", seq);
        if (strlen(seq_rtcp) < 1000) {
          strcat(seq_rtcp, seq_temp);
        }
      }
      TRC_NET("RTCP_S_NACK streamid %s ssrc %u seq %d payload %d seqnumber %s", _parent->_sid.unparse().c_str(), _ssrc, 0, get_payload_type(), seq_rtcp);

      if (_payload_type == RTP_AV_H264) {
        if (((RTPSendTrans*)_parent)->drop_video_rtp_packet()) {
          TRC_NET("RTP_S_NACK_DROP1 streamid %s ssrc %u seq %d payload %d seqnumber %s", _parent->_sid.unparse().c_str(), _ssrc, 0, (int)get_payload_type(), seq_rtcp);
          RTPSession::on_handle_rtcp(rtcpPacketInformation, now);
        }
      }

      for (std::vector<uint16_t>::iterator it = nack_items.begin(); it != nack_items.end(); it++) {
        uint16_t seqNum = *it;
        bool drop = _config->max_packet_age
          && uint32_t(_lastseq - seqNum) > _config->max_packet_age;
        if (drop) {
          TRC_NET("RTP_S_NACK_DROP2 streamid %s ssrc %u seq %d payload %d", _parent->_sid.unparse().c_str(), _ssrc, (int)seqNum, (int)get_payload_type());
          continue;
        }

        uint16_t packet_len = 0;
        int32_t status_code = 0;
        avformat::RTP_FIXED_HEADER *packet = _parent->get_rtp_by_ssrc_seq(
          _ssrc, seqNum, packet_len, status_code);
        if (packet) {
          uint32_t time_base = _parent->get_video_time_base();
          if (packet->payload == RTP_AV_AAC) {
            time_base = _parent->get_audio_time_base();
          }
          if (time_base > 0 && abs(int(_last_rtp_timestamp
            - packet->get_rtp_timestamp())) >= int(20 * time_base)) {
            continue;
          }

          unsigned int max_retrans_count = 100;
          if (packet->payload == RTP_AV_H264) {
            bool key_frame = (packet_len > 18) && (((unsigned char *)packet)[18] >> 7);
            if (key_frame) {
              max_retrans_count = 7;
            }
            else {
              max_retrans_count = 5;
            }
          }
          else {
            max_retrans_count = 9;
          }
          uint32_t interval = ((RTPSendTrans*)_parent)->get_rtt_ms() * 2/3;
          if (interval < _config->min_same_nack_interval) {
            interval = _config->min_same_nack_interval;
          }

          tNackRetransInfo &info = _nacks[seqNum];
          if (info.retrans_count < max_retrans_count && now - info.time > interval) {
            int ret = _parent->send_rtp(_ssrc, packet, packet_len);
            if (ret > 0) {
              _speed.add_bytes(ret);
            }
            info.retrans_count++;
            info.time = now;
            TRC_NET("RTP_S_NACK streamid %s ssrc %u seq %d payload %d timestamp %u len %d", _parent->_sid.unparse().c_str(), _ssrc, (int)packet->get_seq(), (int)packet->payload, packet->get_rtp_timestamp(), (int)packet_len);
          }
        }
      }
    }

    if (rtcpPacketInformation->rtcpPacketTypeFlags & avformat::kRtcpRr) {
      avformat::ReportBlockList report_blocks = rtcpPacketInformation->report_blocks;
      uint32_t max_rtt = 0;
      avformat::ReportBlockList::iterator it;
      for (it = report_blocks.begin(); it != report_blocks.end(); it++) {
        avformat::RTCPReportBlock report_block = *(it);
        uint32_t delay_rr_ms = report_block.delaySinceLastSR >> 16;
        uint32_t time_used = uint32_t(_m_clock->CurrentNtpInMilliseconds()) - report_block.lastSR;
        TRC_NET("RTCP_S_RTT streamid %s ssrc %u delay_rr_ms %d time_used %d lastSR %d ", _parent->_sid.unparse().c_str(), _ssrc, delay_rr_ms, (int)time_used, report_block.lastSR);
        uint32_t rtt = 0;
        if (time_used > delay_rr_ms && report_block.lastSR > 0) {
          rtt = time_used - delay_rr_ms;
        }

        max_rtt = rtt > max_rtt ? rtt : max_rtt;
        _frac_packet_lost_rate = report_block.fractionLost;
        _total_lost_packet_count = report_block.cumulativeLost;
      }
      _rtt_ms = max_rtt;
      TRC_NET("RTCP_S_RTT streamid %s ssrc %u _rtt_ms %u lost %f paylod %d", _parent->_sid.unparse().c_str(), _ssrc, _rtt_ms, _frac_packet_lost_rate / 255.0f, (int)_payload_type);
      //if (_payload_type == 97) {
      //  char tmp[128];
      //  _snprintf_s(tmp, sizeof(tmp), sizeof(tmp)-1, ".............................RTCP_S_RTT, type=%d, rtt=%u, lost=%f\n", (int)_payload_type, _rtt_ms, _frac_packet_lost_rate / 255.0f);
      //  OutputDebugStringA(tmp);
      //}

#ifndef RTP_SERVER
      rtcpPacketInformation->rtt = _rtt_ms;
      _bandwidth->OnReceivedRtcpReceiverReport(
        rtcpPacketInformation->report_blocks,
        rtcpPacketInformation->rtt,
        now);
#endif
    }

    if (rtcpPacketInformation->rtcpPacketTypeFlags & avformat::kRtcpRemb) {
      _receiver_bitrate = rtcpPacketInformation->receiverEstimatedMaxBitrate;
    }

    RTPSession::on_handle_rtcp(rtcpPacketInformation, now);
  }

  void RTPSendSession::set_expect_bitrate(uint32_t bitrate) {
    TRC("RTPSendSession::set_expect_bitrate, bitrate=%u", bitrate);
    uint32_t min_bitrate = bitrate / 10;
    if (min_bitrate < 50000) {
      if (bitrate > 50000)
        min_bitrate = 50000;
      else
        min_bitrate = bitrate;
    }
    _bandwidth->SetMinMaxBitrate(min_bitrate, bitrate);
    _bandwidth->SetSendBitrate(bitrate);
  }

  void RTPSendSession::set_send_bitrate(uint32_t bitrate) {
    _bandwidth->SetSendBitrate(bitrate);
  }

  bool RTPSendSession::get_bandwidth_estimate(uint32_t* bitrate, uint8_t* loss, int64_t* rtt) {
    if (is_packet_lost_rate_valid()) {
      _bandwidth->CurrentEstimate(bitrate, loss, rtt);
      return true;
    }
    else {
      return false;
    }
  }

}
