#pragma once

#include "../engine_api/rtp_upload.h"
#include "../upload/rtp_package.h"
#include <pthread.h>
#include <event.h>
#include <memory>
#include <string>
#include <deque>

#ifndef WIN32
#ifndef PTW32_CDECL
#define PTW32_CDECL
#endif
#endif

class CHttpFetch;
class CEventTask;

namespace media_manager {
  class RTPMediaCache;
}

namespace network {
  class BaseNetworkChannel;
}

namespace live_stream_sdk {
  class DataBuffer;
  class RTPSendTrans;

  class RTPUploadInternal {
  public:
    RTPUploadInternal(RtpNetworkObserver* observer);
    ~RTPUploadInternal();

    int32_t Start(const rtcUploadDispatchConfig* config);
    int32_t Stop();
    int32_t SendAAC(char *data, int len, unsigned int timestamp_ms);
    int32_t SendH264(char *data, int len, int width, int height, unsigned int timestamp_ms);
    RtcCaptureState GetState();

    int32_t SetNetworkChanged();

    void SetAudioParam(unsigned int frequence, unsigned int frame_size);
    void GetSSRC(uint32_t &audio, uint32_t &video);

    void GetVideoEstimate(uint32_t* bitrate, uint8_t* loss, int64_t* rtt);
    void SetVideoExpectBitrate(uint32_t bitrate);

    int SetLostRate(uint32_t send_lost_rate = 0, uint32_t recv_lost_rate = 0);
    RTPTransConfig* GetRTPTransConfig();
    void EnableFec(bool enable);
    void EnableNack(bool enable);
    void GetDispatchConfig(rtcUploadDispatchConfig *config);
    void GetNetworkState(UploadNetworkState* statistic);

  protected:
    void SendPacket();
    int32_t SendRTP(const uint8_t* RTPPacket, uint16_t len);
    int32_t _begin_connect();
    int32_t _send_upload_request();
    int32_t _receive_upload_ack();
    int32_t _receive_rtcp();

    void _on_recv_packet(live_stream_sdk::DataBuffer* buffer);
    void _on_recv_rtp_packet(live_stream_sdk::DataBuffer* buffer);

    std::shared_ptr<media_manager::RTPMediaCache> _rtp_media_cache;

    std::shared_ptr<live_stream_sdk::DataBuffer> _send_data_buffer;
    std::shared_ptr<live_stream_sdk::DataBuffer> _recv_data_buffer;

    int          _estimate_bitrate;

    std::shared_ptr<RTPTransConfig> _rtp_trans_config;

    uint32_t _video_expect_bitrate;

    std::shared_ptr<network::BaseNetworkChannel> _channel;
    int _send_lost_rate;
    int _recv_lost_rate;

    RtpNetworkObserver* _network_observer;

  private:
    static void * PTW32_CDECL WorkerThread(void *arg);
    void* WorkerThreadImpl();
    void ClearWorkerThread();

    static void OnSocketWritable(evutil_socket_t, short, void *);
    static void OnSocketReadable(evutil_socket_t, short, void *);
    static void OnSocketClosed(evutil_socket_t, short, void *);
    static void OnTransTimer(evutil_socket_t, short, void *);
    static void OnSendData(evutil_socket_t, short, void *);
    void OnSocketWritableImpl(evutil_socket_t, short);
    void OnSocketReadableImpl(evutil_socket_t, short);
    void OnSocketClosedImpl(evutil_socket_t, short);
    void OnTransTimerImpl(evutil_socket_t, short);
    void OnSendDataImpl(evutil_socket_t, short);

    void GetUploadUrl();
    void OnGetUploadUrlFinish(int httpcode, const char* data, int len);
    void OnGetUploadUrlSuccessed();
    void OnGetUploadUrlFailed(int httpcode);

    void GetMcuInfo();
    void OnGetMcuInfoFinish(int httpcode, const char* data, int len);
    void OnGetMcuInfoSuccessed();
    void OnGetMcuInfoFailed(int httpcode);

    void SendSdp();
    void OnSendSdpFinish(int httpcode, const char* data, int len);
    void OnSendSdpSuccessed();
    void OnSendSdpFailed(int httpcode);

    void Retry();

    void SetState(RtcCaptureState state, int reason = 0);

    pthread_t m_worker_thread;
    bool m_quit_thread;

    bool m_udp_available;
    unsigned int m_udp_connect_count;

    rtcUploadDispatchConfig m_user_config;
    rtcUploadDispatchConfig m_internal_config;

    event_base *m_ev_base;
    event *m_ev_writable;
    event *m_ev_readable;
    event *m_ev_closed;
    event *m_ev_trans_timer;
    event *m_ev_send;
    CHttpFetch *m_http_upload_url;
    CHttpFetch *m_http_mcu;
    CHttpFetch *m_http_sdp;
    CEventTask *m_retry_task;
    bool m_net_writable;

    unsigned int m_audio_ssrc;
    unsigned int m_video_ssrc;
    std::string m_sdp;

    struct EncodedFrame {
      char *buf;
      unsigned int len;
      unsigned int ts_ms;
      bool keyframe;
      bool video;
      int width;
      int height;
      EncodedFrame(char *buf2, unsigned int len2, unsigned int ts_ms2, bool keyframe2, bool video2, int width2 = 0, int height2 = 0)
        : buf(buf2), len(len2), ts_ms(ts_ms2), keyframe(keyframe2), video(video2), width(width2), height(height2) {}
      EncodedFrame() {
        memset(this, 0, sizeof(EncodedFrame));
      }
    };
    std::deque<EncodedFrame> m_packets;
    pthread_mutex_t m_mutex;

    rtcRTPPackage *_rtp_package;
    RTPSendTrans* _rtp_trans;

    UploadNetworkState m_network_statistic;

    unsigned int m_audio_frequence;
    unsigned int m_audio_frame_size;

    // m_state是给外部看到的状态，m_state_internal则是内部更精细化的状态=RtcCaptureState+InternalState
    RtcCaptureState m_state;
    enum InternalState {
      INTERNAL_STATE_GET_UPLOAD_URL,
      INTERNAL_STATE_GET_MCU_INFO,
      INTERNAL_STATE_SEND_SDP,
      INTERNAL_STATE_MCU_CONNECTING,
      INTERNAL_STATE_MCU_ACKING,
    };
    int m_state_internal;
  };

}
