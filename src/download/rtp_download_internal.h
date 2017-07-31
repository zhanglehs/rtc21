#pragma once

#include "../engine_api/rtp_download.h"
#include "../engine_api/RtcPlayer.h"
#include <pthread.h>
#include <event.h>
#include <memory>
#include <string>

#ifndef WIN32
#ifndef PTW32_CDECL
#define PTW32_CDECL
#endif
#endif

struct event_base;
class CHttpFetch;
class CEventTask;

namespace media_manager {
  class RTPMediaCache;
}

namespace network {
  class BaseNetworkChannel;
}

namespace live_stream_sdk {
  class RTPRecvTrans;
  class DataBuffer;
  class RtpNetworkObserver;
  class RTPTransConfig;
  struct ReceiverReport;

  class RTPDownloadInternal {
  public:
    RTPDownloadInternal(RtpNetworkObserver* observer);
    ~RTPDownloadInternal();

    int Start(const rtcDownloadDispatchConfig *config);
    void Stop();
    RtcPlayerState GetState();

    void SetNetworkChanged();

    RTPTransConfig* GetRTPTransConfig();
    void EnableFec(bool enable);
    void EnableNack(bool enable);
    // 调试接口，有线程安全问题
    void GetDispatchConfig(rtcDownloadDispatchConfig *config);
    // 调试接口
    int SetLostRate(uint32_t send_lost_rate = 0, uint32_t recv_lost_rate = 0);
    void GetNetworkState(DownloadNetworkState* state);

  private:
    static void * PTW32_CDECL WorkerThread(void *arg);
    void* WorkerThreadImpl();
    void ClearWorkerThread();

    static void OnSocketWritable(evutil_socket_t, short, void *);
    static void OnSocketReadable(evutil_socket_t, short, void *);
    static void OnSocketClosed(evutil_socket_t, short, void *);
    static void OnTransTimer(evutil_socket_t, short, void *);

    void OnSocketWritableImpl(evutil_socket_t, short);
    void OnSocketReadableImpl(evutil_socket_t, short);
    void OnSocketClosedImpl(evutil_socket_t, short);
    void OnTransTimerImpl(evutil_socket_t, short);

    void GetPlaylist();
    void OnGetPlaylistFinish(int httpcode, const char* data, int len);
    void OnGetPlaylistSuccessed();
    void OnGetPlaylistFailed(int httpcode);
    void GetSdp();
    void OnGetSdpFinish(int httpcode, const char* data, int len);
    void OnGetSdpSuccessed();
    void OnGetSdpFailed(int httpcode);

    void Retry();

    void SetState(RtcPlayerState state, int reason = 0);

    rtcDownloadDispatchConfig m_user_config;
    rtcDownloadDispatchConfig m_internal_config;
    std::string m_sdp;

    event_base *m_ev_base;
    event *m_ev_writable;
    event *m_ev_readable;
    event *m_ev_closed;
    event *m_ev_trans_timer;

    pthread_t m_worker_thread;
    bool m_quit_thread;
    CHttpFetch *m_http_playlist;
    CHttpFetch *m_http_sdp;
    CEventTask *m_retry_task;

    bool m_udp_available;
    unsigned int m_udp_connect_count;

    RtcPlayerState m_state;
    enum InternalState {
      INTERNAL_STATE_GET_MCU_INFO,
      INTERNAL_STATE_GET_SDP,
      INTERNAL_STATE_MCU_CONNECTING,
      INTERNAL_STATE_MCU_ACKING,
    };
    int m_state_internal;

  protected:
    static void OnFecRecoverPacket(uint8_t* RTPPacket, uint16_t len, void *arg);
    void OnFecRecoverPacketImpl(uint8_t *data, uint16_t len);

    int32_t _setup_channel();
    int32_t _send_udp_req();
    int32_t _recving_rtp();

    void _on_recv_packet(live_stream_sdk::DataBuffer* buffer);
    void _on_recv_rtp_packet(live_stream_sdk::DataBuffer* buffer);
    void _on_recv_rtcp_packet(live_stream_sdk::DataBuffer* buffer);

  private:
    RTPRecvTrans  *_rtp_trans;
    DownloadNetworkState m_network_statistic;

    live_stream_sdk::DataBuffer* _buffer;

    std::shared_ptr<RTPTransConfig> _rtp_trans_config;

    std::shared_ptr<media_manager::RTPMediaCache> _rtp_media_cache;

    std::shared_ptr<network::BaseNetworkChannel> _channel;
    int _send_lost_rate;
    int _recv_lost_rate;

    RtpNetworkObserver* _network_observer;
  };

}
