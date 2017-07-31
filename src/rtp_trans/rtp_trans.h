/*
* Author: gaosiyu@youku.com, hechao@youku.com
*/

#pragma once

#include "../avformat/rtp.h"
#include "../sdk/streamid.h"
#include "../network/network_channel.h"
#include "../sdp/sdp.h"
#include <pthread.h>
#include <memory>
#include <string>
#include <map>

namespace media_manager {
  class RTPMediaCache;
}

namespace avformat {
  class RtcpPacket;
  class Clock;
  class RTCP;
}

namespace live_stream_sdk {
  enum RTPTransMode {
    SENDER_TRANS_MODE,
    RECEIVER_TRANS_MODE
  };

  typedef void(*OnFECRTP_t)(uint8_t* RTPPacket, uint16_t len, void *arg);

  class RTPTransConfig;
  class RTPSession;
  class DataBuffer;

  class RTPTrans {
  public:
    RTPTrans(StreamId_Ext sid, RTPTransMode mode, std::shared_ptr<RTPTransConfig> config);
    virtual ~RTPTrans();

    virtual void on_handle_rtp(const uint8_t* data, uint32_t pkt_len);
    virtual bool on_handle_rtcp(const uint8_t *data, uint32_t data_len, int *payloadtype);

    virtual RTPSession* create_session(RTPTransConfig* config, uint32_t ssrc, RTPAVType payload_type) = 0;

    virtual void on_timer();

    virtual void destroy();

    virtual void reset(StreamId_Ext id);
    bool is_alive();

    const StreamId_Ext &get_sid() const { return _sid; }

    uint32_t get_video_ssrc();
    uint32_t get_audio_ssrc();
    uint32_t get_video_frac_packet_lost_rate();
    uint32_t get_audio_frac_packet_lost_rate();
    uint32_t get_video_total_lost_packet_count();
    uint32_t get_audio_total_lost_packet_count();
    uint32_t get_video_total_rtp_packetcount();
    uint32_t get_audio_total_rtp_packetcount();
    uint32_t get_video_rtt_ms();
    uint32_t get_audio_rtt_ms();
    uint32_t get_video_current_bitrate();
    uint32_t get_audio_current_bitrate();
    uint32_t get_video_effect_bitrate();
    uint32_t get_audio_effect_bitrate();
    uint64_t get_video_total_rtp_bytes();
    uint64_t get_audio_total_rtp_bytes();

    void set_rtp_cache(media_manager::RTPMediaCache* media_cache) { _media_cache = media_cache; }
    media_manager::RTPMediaCache* get_rtp_cache() { return _media_cache; }
    void set_time_base(uint32_t audio_time_base, uint32_t video_time_base);
    uint32_t get_audio_time_base() { return _audio_time_base; }
    uint32_t get_video_time_base() { return _video_time_base; }

  public:
    int32_t set_channel(std::shared_ptr<network::BaseNetworkChannel>);

    void RegisterOnFecRtpCallback(live_stream_sdk::OnFECRTP_t fec_rtp_callback, void *arg) {
      _fec_rtp_callback = fec_rtp_callback;
      _fecdata = arg;
    }

  public:
    StreamId_Ext _sid;
    virtual int send_rtcp(uint32_t ssrc, const avformat::RtcpPacket *pkt);
    virtual int send_rtp(uint32_t ssrc, const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len);
    virtual int recv_rtp(uint32_t ssrc, const avformat::RTP_FIXED_HEADER *pkt, uint32_t pkt_len);
    virtual avformat::RTP_FIXED_HEADER* get_rtp_by_ssrc_seq(uint32_t ssrc, uint16_t seq, uint16_t &len, int32_t& status_code);

  protected:
    RTPSession *get_video_session();
    RTPSession *get_audio_session();
    void clear_sessions();

  protected:
    pthread_mutex_t _mutex;
    uint64_t _init_ts;
    avformat::Clock* _m_clock;
    avformat::RTCP *_rtcpParser;

    std::map<uint32_t, RTPSession *> _sessions;
    std::shared_ptr<RTPTransConfig> _config;

    media_manager::RTPMediaCache* _media_cache;

    bool _is_alive;

    std::shared_ptr<network::BaseNetworkChannel> _channel;

    live_stream_sdk::DataBuffer* _buffer;

    live_stream_sdk::OnFECRTP_t _fec_rtp_callback;
    void *_fecdata;

    std::string _last_error;
    RTPTransMode _mode;
    uint32_t _audio_time_base;
    uint32_t _video_time_base;
  };

}
