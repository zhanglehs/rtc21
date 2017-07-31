#pragma once

#include "RtcPlayer.h"
#include <stdint.h>
#include <string.h>

#ifndef DLLEXPORT
#ifdef WIN32
#ifdef ENGINE_SDK_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif
#else
#define DLLEXPORT
#endif
#endif

struct rtcDownloadDispatchConfig {
  char appid[32];
  char alias[64];
  char lapi_host[256];
  char lapi_token[128];

  bool is_tcp;
  bool disable_playlist;
  char streamid[64];
  char mcu_ip[128];
  unsigned short mcu_udp_port;
  unsigned short mcu_tcp_port;
  char mcu_token[128];
  char sdp_url[1024];

  rtcDownloadDispatchConfig() {
    memset(this, 0, sizeof(rtcDownloadDispatchConfig));
  }
};

namespace live_stream_sdk {
  class RTPTransConfig;
  class RTPDownloadInternal;
  class RtpNetworkObserver;

  struct ReceiverReport {
    uint32_t  frac_lost_packet_count;
    uint32_t total_lost_packet_count;
    uint32_t total_rtp_packet_count;
    uint32_t effect_bitrate;
    uint32_t total_bitrate;
    uint64_t total_recvbytes;
    float packet_lost_rate;
  };

  struct DownloadNetworkState {
    unsigned int audio_total_recv_packet_count;
    unsigned int audio_total_lost_packet_count;
    unsigned int audio_bps;
    unsigned int audio_total_recv_bytes;
    unsigned int video_total_recv_packet_count;
    unsigned int video_total_lost_packet_count;
    unsigned int video_bps;
    unsigned int video_total_recv_bytes;
    unsigned int packet_lost_percent;
    bool is_tcp;
  };

  class DLLEXPORT RTPDownload {
  public:
    RTPDownload(const char* deviceid, RtpNetworkObserver* observer = NULL);
    ~RTPDownload();
    int Start(const rtcDownloadDispatchConfig *config);
    int32_t Stop();
    RtcPlayerState GetState();

    int32_t SetNetworkChanged();

    void GetDispatchConfig(rtcDownloadDispatchConfig *config);
    RTPTransConfig* GetRTPTransConfig();
    void EnableFec(bool enable);
    void EnableNack(bool enable);
    void GetNetworkState(DownloadNetworkState* state);
    void DebugLost(uint32_t send_lost_rate, uint32_t recv_lost_rate);

  private:
    RTPDownloadInternal* m_internal;
  };
}
