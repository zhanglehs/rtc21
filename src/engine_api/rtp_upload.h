#pragma once

#include "RtcCapture.h"
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

namespace live_stream_sdk {
  class RTPTransConfig;
  class RTPUploadInternal;
  class RtpNetworkObserver;

  typedef void(*rtcUploadOnMessageCallback)(unsigned int msgid, const char* content);

  struct UploadNetworkState {
    unsigned int audio_total_send_packet_count;
    unsigned int audio_total_lost_packet_count;
    unsigned int audio_bps;
    unsigned int audio_total_bytes;
    unsigned int video_total_send_packet_count;
    unsigned int video_total_lost_packet_count;
    unsigned int video_bps;
    unsigned int video_total_bytes;
    unsigned int packet_lost_percent;
    unsigned int rtt_ms;
    bool is_tcp;
  };

  struct rtcUploadDispatchConfig {
    char appid[32];
    char alias[64];
    char lapi_host[256];
    char lapi_token[128];

    bool is_tcp;
    char streamid[64];
    char upload_url[1024];
    char mcu_ip[128];
    unsigned short mcu_udp_port;
    unsigned short mcu_tcp_port;
    char mcu_token[128];
    char sdp_url[1024];

    rtcUploadDispatchConfig() {
      memset(this, 0, sizeof(rtcUploadDispatchConfig));
    }
  };

  class DLLEXPORT RTPUpload {
  public:
    RTPUpload(const char* deviceid, RtpNetworkObserver* observer = NULL);
    ~RTPUpload();

    int32_t Start(const rtcUploadDispatchConfig* config);
    int32_t Stop();
    int32_t SendAAC(char *data, int len, unsigned int timestamp_ms);
    int32_t SendH264(char *data, int len, int width, int height, unsigned int timestamp_ms);
    RtcCaptureState GetState();

    // 调用者检测到断网恢复后，调用该函数能加快恢复速度
    int32_t SetNetworkChanged();

    void SetAudioParam(unsigned int frequence, unsigned int frame_size);
    void GetSSRC(uint32_t &audio, uint32_t &video);

    void SetVideoExpectBitrate(uint32_t bitrate);
    void GetVideoEstimate(uint32_t* bitrate, uint8_t* loss, int64_t* rtt);

    // 以下接口为调试用途
    void DebugLost(uint32_t send_lost_rate, uint32_t recv_lost_rate);
    RTPTransConfig* GetRTPTransConfig();
    void EnableFec(bool enable);
    void EnableNack(bool enable);
    // 有多线程问题，后果是极小概率返回字符数组内容不正确（但能保证有结束符），无崩溃风险
    void GetDispatchConfig(rtcUploadDispatchConfig *config);
    void GetNetworkState(UploadNetworkState* state);

  private:
    RTPUploadInternal* rtp_upload_internal;
  };

}
