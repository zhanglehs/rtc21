#pragma once

#include "../avformat/rtp.h"
#include "../sdp/sdp.h"
#include <map>

namespace avformat {
  class RTCPPacketInformation;
  class RtcpPacket;
  class Clock;
}

namespace live_stream_sdk {
  class RTPTrans;
  struct RTPControlConfig;
  class RTPTransConfig;

  class RTPSessionBitrate {
  public:
    RTPSessionBitrate() : _bytes(0), _last_bytes(0), _ts(0), _speed_kbps(0) {}
    uint32_t get_total_bytes() { return _bytes; }
    uint32_t get_kbps() { return _speed_kbps; }
    void add_bytes(uint32_t bytes) { _bytes += bytes; }
    void update_speed(uint64_t now) {
      if (now - _ts > 1000) {
        _speed_kbps = (_bytes - _last_bytes) / uint32_t(now - _ts) * 8;
        _last_bytes = _bytes;
        _ts = now;
      }
    }

  private:
    uint32_t _bytes;
    uint32_t _last_bytes;
    uint64_t _ts;
    uint32_t _speed_kbps;
  };

  class RTPSession {
  public:
    RTPSession(RTPTrans * trans, RTPTransConfig* config, uint32_t ssrc, RTPAVType payload_type);
    virtual ~RTPSession();
    virtual void on_handle_rtp(const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len, uint64_t now);
    virtual void on_handle_rtcp(avformat::RTCPPacketInformation *rtcpPacketInformation, uint64_t now);
    virtual void on_timer(uint64_t now) = 0;
    void destroy();
    bool check_timeout(uint64_t now);
    bool is_timeout();
    bool is_closed();

    uint32_t get_ssrc();
    uint32_t get_frac_packet_lost_rate() { return _frac_packet_lost_rate; }
    uint32_t get_total_lost_packet_count() { return _total_lost_packet_count; }
    uint32_t get_total_rtp_packetcount() { return _total_rtp_packetcount; }
    uint32_t get_rtt_ms() { return _rtt_ms; }
    uint32_t get_effect_bitrate() { return _receiver_bitrate; }
    uint32_t get_current_bitrate() { return _speed.get_kbps()*1000; }
    uint64_t get_total_rtp_bytes() { return _speed.get_total_bytes(); }
    live_stream_sdk::RTPAVType get_payload_type(){ return _payload_type; }

  protected:
    static bool is_newer_seq(uint16_t seq, uint16_t preseq);
    class SequenceNumberLessThan {
    public:
      bool operator() (const uint16_t& sequence_number1,
        const uint16_t& sequence_number2) const {
        return is_newer_seq(sequence_number2, sequence_number1);
      }
    };
    bool isNacklistToolarge();
    bool isNacklistTooOld();
    virtual void _on_handle_rtp_fec(const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len, uint64_t now) = 0;
    virtual void _on_handle_rtp_nack(const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len, uint64_t now) = 0;
    int fec_xor_32(void * src, uint32_t src_len, void *dst, uint32_t dst_len);
    int fec_xor_64(void * src, uint32_t src_len, void *dst, uint32_t dst_len);
    uint32_t _send_rtcp(uint32_t ssrc, avformat::RtcpPacket *pkt);
    void keepalive(uint64_t now);

  protected:
    uint32_t _ssrc;
    RTPTrans *_parent;
    RTPTransConfig* _config;
    live_stream_sdk::RTPAVType _payload_type;

    uint8_t  _frac_packet_lost_rate;
    RTPSessionBitrate _speed;
    uint32_t _total_rtp_packetcount;//for sender and receiver:only include normal rtp packets
    uint32_t _total_lost_packet_count;
    uint32_t _rtt_ms;
    uint32_t _receiver_bitrate;
    uint64_t _last_process_sr_ts;
    uint16_t _lastseq;
    uint8_t  _pkt_data[2048];
    uint8_t  _xor_tmp_data[2048];
    struct tNackRetransInfo {
      uint64_t time;
      unsigned int retrans_count;
      tNackRetransInfo() : time(0), retrans_count(0) {}
    };
    std::map<uint16_t, tNackRetransInfo, SequenceNumberLessThan> _nacks;
    avformat::Clock* _m_clock;
    bool _timeout;
    bool _closed;
    uint64_t _active_time_ms;
  };
}
