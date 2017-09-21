#include "rtp_download_internal.h"

#include "../avformat/rtcp.h"
#include "../cmd_protocol/proto_define_rtp.h"
#include "../cmd_protocol/proto_rtp_rtcp.h"
#include "../cmd_protocol/proto_common.h"
#include "../log/log.h"
#include "../media_manager/rtp_block_cache.h"
#include "../network/tcp.h"
#include "../network/udp.h"
#include "../util/util_common.h"
#include "../rtp_trans/rtp_receiver_trans.h"
#include "../sdk/sdk_interface.h"
#include "../log/report.h"
#include "../network/CHttpFetch.h"
#include "../engine_api/rtp_download.h"
#include "../sdk/rtp_format.h"
#include <json/json.h>

#ifndef WIN32
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
typedef int SOCKET;
#else
#include <ws2tcpip.h>
#include <Windows.h>
#endif
#include <math.h>
#include <algorithm>
#include <string>
#include <map>
#ifdef __ANDROID__
#include <sys/prctl.h>
#endif

// TODO: zhangle
#include <event2/thread.h>

using namespace avformat;

namespace {
  void libevent_log(int severity, const char *msg) {
  }
}

namespace live_stream_sdk {

  RTPDownloadInternal::RTPDownloadInternal(RtpNetworkObserver* observer) {
    // TODO: zhangle
#ifdef WIN32
    evthread_use_windows_threads();
#else
    evthread_use_pthreads();
#endif
    event_set_log_callback(libevent_log);

    INF("RTPDownloadSDKImpl construct %p", this);

    _network_observer = observer;

    _rtp_trans_config.reset(new RTPTransConfig());

    _buffer = new DataBuffer(1024 * 1024);
    _rtp_trans = NULL;

    _send_lost_rate = 0;
    _recv_lost_rate = 0;

    memset(&m_worker_thread, 0, sizeof(m_worker_thread));
    m_ev_base = NULL;
    m_ev_closed = NULL;
    m_ev_writable = NULL;
    m_ev_readable = NULL;
    m_ev_trans_timer = NULL;
    m_quit_thread = false;
    m_http_playlist = NULL;
    m_http_sdp = NULL;
    m_retry_task = new CEventTask();
    m_udp_available = false;
    m_udp_connect_count = 0;
    m_state = RtcPlayerState::RTC_PLAYER_STATE_UNSET;
    m_state_internal = RtcPlayerState::RTC_PLAYER_STATE_UNSET;
    memset(&m_network_statistic, 0, sizeof(m_network_statistic));
  }

  int RTPDownloadInternal::Start(const rtcDownloadDispatchConfig *config) {
    if (IsThreadIdValid(&m_worker_thread)) {
      // already running
      return -1;
    }

    if (!IsAliasValid(config->alias))  {
      ERR("rtp_download init failed, bad alias");
      return -1;
    }

    m_quit_thread = false;
    m_udp_available = false;
    m_udp_connect_count = 0;

    memcpy(&m_user_config, config, sizeof(m_user_config));
    memcpy(&m_internal_config, config, sizeof(m_internal_config));
    memset(&m_network_statistic, 0, sizeof(m_network_statistic));

    SetState(RtcPlayerState::RTC_PLAYER_STATE_INITIALIZING);
    if (pthread_create(&m_worker_thread, NULL, WorkerThread, this) < 0) {
      SetState(RtcPlayerState::RTC_PLAYER_STATE_ERROR, RtcPlayerErrorType::RTC_PLAYER_ERROR_CREATETHREAD_FAILED);
      SetState(RtcPlayerState::RTC_PLAYER_STATE_STOPPED, RtcPlayerStopType::RTC_PLAYER_STOP_BY_ERROR);
    }
    return 0;
  }

  void RTPDownloadInternal::GetDispatchConfig(rtcDownloadDispatchConfig *config) {
    memcpy(config, &m_internal_config, sizeof(m_internal_config));
  }

  void * PTW32_CDECL RTPDownloadInternal::WorkerThread(void *arg) {
#ifdef __ANDROID__
    prctl(PR_SET_NAME, reinterpret_cast<unsigned long>("NetDownloadThread"));
#endif
    RTPDownloadInternal *pThis = (RTPDownloadInternal*)arg;
    return pThis->WorkerThreadImpl();
  }

  void* RTPDownloadInternal::WorkerThreadImpl() {
    struct event_config *ev_config = event_config_new();
    event_config_set_flag(ev_config, EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);
    m_ev_base = event_base_new_with_config(ev_config);
    event_config_free(ev_config);

    while (!m_quit_thread) {
      GetPlaylist();
      event_base_dispatch(m_ev_base);
      event_base_loopbreak(m_ev_base);
      ClearWorkerThread();
    }

    return NULL;
  }

  void RTPDownloadInternal::OnSocketWritable(evutil_socket_t fd, short flag, void *arg) {
    RTPDownloadInternal *pThis = (RTPDownloadInternal*)arg;
    pThis->OnSocketWritableImpl(fd, flag);
  }

  void RTPDownloadInternal::OnSocketReadable(evutil_socket_t fd, short flag, void *arg) {
    RTPDownloadInternal *pThis = (RTPDownloadInternal*)arg;
    pThis->OnSocketReadableImpl(fd, flag);
  }

  void RTPDownloadInternal::OnSocketClosed(evutil_socket_t fd, short flag, void *arg) {
    RTPDownloadInternal *pThis = (RTPDownloadInternal*)arg;
    pThis->OnSocketClosedImpl(fd, flag);
  }

  void RTPDownloadInternal::OnTransTimer(evutil_socket_t fd, short flag, void *arg) {
    RTPDownloadInternal *pThis = (RTPDownloadInternal*)arg;
    pThis->OnTransTimerImpl(fd, flag);
  }

  void RTPDownloadInternal::OnSocketWritableImpl(evutil_socket_t, short) {
    if (m_state_internal == INTERNAL_STATE_MCU_CONNECTING) {
      _send_udp_req();
      m_ev_readable = event_new(m_ev_base, _channel->get_sock_fd(), EV_READ | EV_PERSIST, OnSocketReadable, this);
      event_add(m_ev_readable, NULL);
      m_ev_trans_timer = event_new(m_ev_base, -1, EV_PERSIST, OnTransTimer, this);
      struct timeval tv;
      tv.tv_sec = 0;
      tv.tv_usec = 10000;
      event_add(m_ev_trans_timer, &tv);
    }
  }

  void RTPDownloadInternal::OnSocketReadableImpl(evutil_socket_t, short) {
    if (m_state_internal == INTERNAL_STATE_MCU_ACKING) {
      SetState(RtcPlayerState::RTC_PLAYER_STATE_RUNNING);
    }
    if (m_state == RtcPlayerState::RTC_PLAYER_STATE_RUNNING) {
      _recving_rtp();
    }
  }

  void RTPDownloadInternal::OnSocketClosedImpl(evutil_socket_t, short) {
    if (m_state == RtcPlayerState::RTC_PLAYER_STATE_RUNNING) {
      SetState(RtcPlayerState::RTC_PLAYER_STATE_ERROR, RtcPlayerErrorType::RTC_PLAYER_ERROR_NETWORK_ERROR);
    }
    m_retry_task->Post(m_ev_base, std::bind(&RTPDownloadInternal::Retry, this), 1000);
  }

  void RTPDownloadInternal::OnTransTimerImpl(evutil_socket_t, short) {
    _rtp_trans->on_timer();
    if (!_rtp_trans->is_alive()) {
      if (m_state == RtcPlayerState::RTC_PLAYER_STATE_RUNNING) {
        SetState(RtcPlayerState::RTC_PLAYER_STATE_ERROR, RtcPlayerErrorType::RTC_PLAYER_ERROR_NETWORK_TIMEOUT);
      }
      Retry();
    }
    else {
      // TODO: zhangle, test
      static int s_count = 0;
      s_count++;
      if (s_count % 10) {
        return;
      }
      m_network_statistic.audio_total_recv_packet_count = _rtp_trans->get_audio_total_rtp_packetcount();
      m_network_statistic.audio_total_lost_packet_count = _rtp_trans->get_audio_total_lost_packet_count();
      m_network_statistic.audio_bps = _rtp_trans->get_audio_current_bitrate();
      m_network_statistic.audio_total_recv_bytes = (unsigned int)_rtp_trans->get_audio_total_rtp_bytes(); // TODO: zhangle
      m_network_statistic.video_total_recv_packet_count = _rtp_trans->get_video_total_rtp_packetcount();
      m_network_statistic.video_total_lost_packet_count = _rtp_trans->get_video_frac_packet_lost_rate();
      m_network_statistic.video_bps = _rtp_trans->get_video_current_bitrate();
      m_network_statistic.video_total_recv_bytes = (unsigned int)_rtp_trans->get_video_total_rtp_bytes(); // TODO: zhangle
      m_network_statistic.packet_lost_percent = _rtp_trans->get_audio_frac_packet_lost_rate() * 100 / 255;
      m_network_statistic.is_tcp = m_internal_config.is_tcp;
    }
  }

  void RTPDownloadInternal::Retry() {
    event_base_loopbreak(m_ev_base);
  }

  void RTPDownloadInternal::SetState(RtcPlayerState state, int reason) {
    m_state = state;
    m_state_internal = state;
    if (_network_observer) {
      _network_observer->OnNetworkState(state, reason);
    }
  }

  void RTPDownloadInternal::ClearWorkerThread() {
    if (_rtp_trans) {
      _rtp_trans->destroy();
      delete _rtp_trans;
      _rtp_trans = NULL;
    }

    if (m_ev_writable) {
      event_free(m_ev_writable);
      m_ev_writable = NULL;
    }
    if (m_ev_readable) {
      event_free(m_ev_readable);
      m_ev_readable = NULL;
    }
    if (m_ev_closed) {
      event_free(m_ev_closed);
      m_ev_closed = NULL;
    }
    if (m_ev_trans_timer) {
      event_free(m_ev_trans_timer);
      m_ev_trans_timer = NULL;
    }

    if (m_http_playlist) {
      CHttpFetch::Destroy(m_http_playlist);
      m_http_playlist = NULL;
    }
    if (m_http_sdp) {
      CHttpFetch::Destroy(m_http_sdp);
      m_http_sdp = NULL;
    }
    m_retry_task->Cancel();

    if (_channel) {
      _channel->close();
    }
  }

  void RTPDownloadInternal::Stop() {
    m_quit_thread = true;
    if (m_ev_base) {
      event_base_loopbreak(m_ev_base);
    }

    if (IsThreadIdValid(&m_worker_thread)) {
      pthread_join(m_worker_thread, NULL);
      memset(&m_worker_thread, 0, sizeof(m_worker_thread));
    }

    if (m_ev_base) {
      event_base_free(m_ev_base);
      m_ev_base = NULL;
    }
    if (m_state != RtcPlayerState::RTC_PLAYER_STATE_UNSET && m_state != RtcPlayerState::RTC_PLAYER_STATE_STOPPED) {
      m_state_internal = RtcPlayerState::RTC_PLAYER_STATE_STOPPED;
      m_state = RtcPlayerState::RTC_PLAYER_STATE_STOPPED;
    }
  }

  RtcPlayerState RTPDownloadInternal::GetState() {
    return m_state;
  }

  int32_t RTPDownloadInternal::_setup_channel() {
    try  {
      bool tcp = false;
      if (m_user_config.is_tcp) {
        tcp = true;
      }
      else if (m_udp_available || m_udp_connect_count < 3) {
        tcp = false;
      }
      else {
        tcp = !m_internal_config.is_tcp;
      }
      m_internal_config.is_tcp = tcp;

      if (tcp)  {
        _channel.reset(new network::CMDTCPChannel());
        INF("download server ip:%s,tcp port:%d.", m_internal_config.mcu_ip, (int)m_internal_config.mcu_tcp_port);
        _channel->init(m_internal_config.mcu_ip, m_internal_config.mcu_tcp_port, true);
      }
      else  {
        m_udp_connect_count++;
        _channel.reset(new network::SimpleUDPChannel());
        INF("download server ip:%s,udp port:%d.", m_internal_config.mcu_ip, (int)m_internal_config.mcu_udp_port);
        _channel->init(m_internal_config.mcu_ip, m_internal_config.mcu_udp_port, true);
      }

      _channel->set_lost_rate(_send_lost_rate, _recv_lost_rate);
    }
    catch (BaseException& ex) {
      ERR(ex.GetErrorMessage().c_str());
      m_retry_task->Post(m_ev_base, std::bind(&RTPDownloadInternal::Retry, this), 1000);
      return -1;
    }

    m_state_internal = InternalState::INTERNAL_STATE_MCU_CONNECTING;
    INF("setup channel success, waiting for connected.");
    _rtp_trans->set_channel(_channel);

    m_ev_closed = event_new(m_ev_base, _channel->get_sock_fd(), EV_CLOSED | EV_PERSIST, OnSocketClosed, this);
    m_ev_writable = event_new(m_ev_base, _channel->get_sock_fd(), EV_WRITE, OnSocketWritable, this);
    event_add(m_ev_closed, NULL);
    event_add(m_ev_writable, NULL);

    return 0;
  }

  void RTPDownloadInternal::GetPlaylist() {
    memcpy(&m_internal_config, &m_user_config, sizeof(m_internal_config));
    if (m_user_config.mcu_udp_port > 0 && m_user_config.sdp_url[0]) {
      OnGetPlaylistSuccessed();
      return;
    }
    m_state_internal = INTERNAL_STATE_GET_MCU_INFO;
    char url[1024];
    memset(url, 0, sizeof(url));
    snprintf(url, sizeof(url),
      "http://%s/v3/get_playlist?app_id=%s&player_type=app&alias=%s&token=%s",
      m_user_config.lapi_host,
      m_user_config.appid, m_user_config.alias, m_user_config.lapi_token);

    m_http_playlist = CHttpFetch::Create(m_ev_base);
    m_http_playlist->Get(url, std::bind(&RTPDownloadInternal::OnGetPlaylistFinish, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  }

  void RTPDownloadInternal::OnGetPlaylistFinish(int code, const char* data, int len) {
    if (code >= 200 && code < 300) {
      if (data && len > 0) {
        bool success = false;
        int error_code = -1;

        std::string stream_id;
        uint16_t tcp_port = 0;
        std::string ip;
        uint16_t udp_port = 0;
        std::string token;
        std::string sdp;

        try {
          Json::Reader jreader;
          Json::Value jroot;
          jreader.parse(data, data + len, jroot, false);
          if (jroot.isObject() && !jroot.isNull()) {
            Json::Value jcode = jroot["error_code"];
            Json::Value jurls = jroot["url_list"];

            if (jcode.isNumeric()) {
              error_code = jcode.asInt();
            }
            if (error_code == 0 && jurls.isArray() && !jurls.isNull()) {
              for (auto it = jurls.begin(); it != jurls.end(); it++) {
                Json::Value jformat = (*it)["format"];
                if (jformat.isString() && jformat.asString() == "sdp") {
                  Json::Value jstream = (*it)["stream_id"];
                  Json::Value jtcp = (*it)["tcp_port"];
                  Json::Value judp = (*it)["udp_port"];
                  Json::Value jip = (*it)["ip"];
                  Json::Value jtoken = (*it)["play_token"];
                  Json::Value jsdp = (*it)["sdp_url"];
                  if (jstream.isString()) {
                    stream_id = jstream.asCString();
                  }
                  if (jtcp.isNumeric()) {
                    tcp_port = (uint16_t)jtcp.asUInt();
                  }
                  if (judp.isNumeric()) {
                    udp_port = (uint16_t)judp.asUInt();
                  }
                  if (jip.isString()) {
                    ip = jip.asCString();
                  }
                  if (jsdp.isString()) {
                    sdp = jsdp.asCString();
                  }
                  if (jtoken.isString()) {
                    token = jtoken.asCString();
                  }
                  success = true;
                  break;
                }
                else {
                  continue;
                }
              }
            }
          }
          else {
            ERR("parse get play list failed, response: %s", data);
          }
        }
        catch (std::exception &) {
          ERR("parse get play list failed, response: %s", data);
        }

        if (success) {
          if (m_internal_config.streamid[0] == 0) {
            strcpy(m_internal_config.streamid, stream_id.c_str());
          }
          if (m_internal_config.mcu_ip[0] == 0) {
            strcpy(m_internal_config.mcu_ip, ip.c_str());
          }
          if (m_internal_config.sdp_url[0] == 0) {
            strcpy(m_internal_config.sdp_url, sdp.c_str());
          }
          if (m_internal_config.mcu_token[0] == 0) {
            strcpy(m_internal_config.mcu_token, token.c_str());
          }
          if (m_internal_config.mcu_udp_port == 0) {
            m_internal_config.mcu_udp_port = udp_port;
          }
          if (m_internal_config.mcu_tcp_port == 0) {
            m_internal_config.mcu_tcp_port = tcp_port;
          }

          if (!sdp.empty()) {
            OnGetPlaylistSuccessed();
            return;
          }
        }
      }
    }
    else {
      if (data && len > 0) {
        ERR("get play list failed, response: %s", data);
      }
    }

    OnGetPlaylistFailed(code);
  }

  void RTPDownloadInternal::OnGetPlaylistSuccessed() {
    GetSdp();
  }

  void RTPDownloadInternal::OnGetPlaylistFailed(int httpcode) {
    if (httpcode >= 200 && httpcode < 600) {
      // it is failed, and quit thread
      m_quit_thread = true;
      event_base_loopbreak(m_ev_base);
      SetState(RtcPlayerState::RTC_PLAYER_STATE_ERROR, RtcPlayerErrorType::RTC_PLAYER_ERROR_GETMCUINFO_FAILED);
      SetState(RtcPlayerState::RTC_PLAYER_STATE_STOPPED, RtcPlayerStopType::RTC_PLAYER_STOP_BY_ERROR);
    }
    else {
      m_retry_task->Post(m_ev_base, std::bind(&RTPDownloadInternal::Retry, this), 1000);
    }
  }

  void RTPDownloadInternal::GetSdp() {
    if (m_internal_config.sdp_url[0]) {
      m_state_internal = INTERNAL_STATE_GET_SDP;
      m_http_sdp = CHttpFetch::Create(m_ev_base);
      m_http_sdp->Get(m_internal_config.sdp_url, std::bind(&RTPDownloadInternal::OnGetSdpFinish, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }
    else {
      assert(false);
    }
  }

  void RTPDownloadInternal::OnGetSdpFinish(int code, const char* data, int len) {
    if (code >= 200 && code < 300 && data && len > 0) {
      m_sdp = data;
      OnGetSdpSuccessed();
      return;
    }
    OnGetSdpFailed(code);
  }

  void RTPDownloadInternal::OnGetSdpSuccessed() {
    StreamId_Ext sid;
    sid.parse(m_internal_config.streamid);
    _rtp_trans = new RTPRecvTrans(sid, _rtp_trans_config);
    _rtp_trans->RegisterOnFecRtpCallback(OnFecRecoverPacket, this);

    _rtp_media_cache.reset(new media_manager::RTPMediaCache(sid));
    _rtp_media_cache->set_sdp(m_sdp);
    _rtp_trans->set_rtp_cache(_rtp_media_cache.get());

    if (_network_observer) {
      _network_observer->OnReceivedSDP(m_sdp.c_str());
    }
    _setup_channel();
  }

  void RTPDownloadInternal::OnGetSdpFailed(int httpcode) {
    // 返回404仍然重试是对服务器问题的兼容
    if (httpcode >= 200 && httpcode < 600 && httpcode != 404) {
      // it is failed, and quit thread
      m_quit_thread = true;
      event_base_loopbreak(m_ev_base);
      SetState(RtcPlayerState::RTC_PLAYER_STATE_ERROR, RtcPlayerErrorType::RTC_PLAYER_ERROR_GETSDP_FAILED);
      SetState(RtcPlayerState::RTC_PLAYER_STATE_STOPPED, RtcPlayerStopType::RTC_PLAYER_STOP_BY_ERROR);
    }
    else {
      m_retry_task->Post(m_ev_base, std::bind(&RTPDownloadInternal::Retry, this), 1000);
    }
  }

  int32_t RTPDownloadInternal::_send_udp_req() {
    rtp_d2p_req_state req;
    memset(&req, 0, sizeof(req));
    req.payload_type = PAYLOAD_TYPE_RTP;
    StreamId_Ext sid;
    sid.parse(m_internal_config.streamid);
    memcpy(req.streamid, &sid, sizeof(sid));
    memcpy(req.token, m_internal_config.mcu_token, strlen(m_internal_config.mcu_token));
    req.version = 1;
    strcpy((char*)req.useragent, "live_stream_sdk 1.0");

    //NOTE deviceid must empty until server update
    _buffer->clear();
    encode_rtp_d2p_req_state_ext(&req, GetLiveSDKDeviceID(), _buffer);

    try {
      _channel->send_data((char*)_buffer->data_ptr(), static_cast<uint32_t>(_buffer->data_len()));
      INF("CMD_RTP_D2P_REQ_STATE send out.");
    }
    catch (BaseException& ex) {
      ERR("network error, reconnect, %s", ex.GetErrorMessage().c_str());
      return -1;
    }

    m_state_internal = INTERNAL_STATE_MCU_ACKING;
    return 0;
  }

  int32_t RTPDownloadInternal::_recving_rtp() {
    uint8_t buffer[256 * 256];

    while (true) {
      int len = _channel->receive_data((char*)buffer, sizeof(buffer));
      if (len < 0) {
        // error
        break;
      }
      if (len == 0) {
        break;
      }
      if (!m_internal_config.is_tcp) {
        m_udp_available = true;
      }
      _buffer->clear();
      _buffer->append_ptr(buffer, len);
      _on_recv_packet(_buffer);
    }

    return 0;
  }

  void RTPDownloadInternal::_on_recv_packet(DataBuffer* buffer) {
    int ret = 0;
    proto_header h;
    h.cmd = 0;
    decode_header(buffer, &h);
    StreamId_Ext stream_id;
    switch (h.cmd) {
    case CMD_RTP_D2P_PACKET:
      ret = decode_rtp_d2p_packet(stream_id, buffer);
      if (ret < 0) {
        ERR("decode rtcp packet error ret code %d", ret);
        break;
      }

      if (stream_id.unparse() == m_internal_config.streamid) {
        _on_recv_rtp_packet(buffer);
      }
      break;
    case CMD_RTCP_D2P_PACKET:
    case CMD_RTCP_U2R_PACKET:
      ret = decode_rtcp_d2p_packet(stream_id, buffer);
      if (ret < 0) {
        ERR("decode rtcp packet error ret code %d", ret);
        break;
      }

      if (stream_id.unparse() == m_internal_config.streamid) {
        _on_recv_rtcp_packet(buffer);
      }
      break;
    default:
      break;
    }
  }

  void RTPDownloadInternal::_on_recv_rtp_packet(DataBuffer* buffer) {
    int len = static_cast<int>(buffer->data_len());
    if (len <= 0) {
      return;
    }

    RTP_FIXED_HEADER* rtp = (RTP_FIXED_HEADER*)buffer->data_ptr();
    RTPAVType payload_type = (RTPAVType)rtp->payload;
    if (len > 2000) {
      ERR("RTP_RECV, packet len > 2000, alias=%s, payload=%d, seqNum=%d, timestamp=%u, streamid=%s, len=%d",
        m_internal_config.alias, (int)payload_type, (int)rtp->get_seq(),
        rtp->get_rtp_timestamp(), m_internal_config.streamid, len);
      return;
    }
    TRC_NET("RTP_R_RTP streamid %s ssrc %u seq %d payload %d timestamp %u len %d",
      m_internal_config.streamid, rtp->get_ssrc(), rtp->get_seq(), rtp->payload, rtp->get_rtp_timestamp(), len);

    if (payload_type == RTP_AV_H264 || payload_type == RTP_AV_AAC) {
      int32_t status = 0;
      _rtp_trans->get_rtp_cache()->set_rtp(rtp, len, status);

      if (_network_observer) {
        _network_observer->OnReceivedRTP((unsigned char*)buffer->data_ptr(), len, payload_type);
      }

      _rtp_trans->on_handle_rtp((const uint8_t*)buffer->data_ptr(), len);
    }
  }

  void RTPDownloadInternal::_on_recv_rtcp_packet(DataBuffer* buffer) {
    int len = static_cast<int>(buffer->data_len());
    uint8_t* data = (uint8_t*)buffer->data_ptr();
    if (len > 0) {
      int payloadtype = 0;
      bool isSR = _rtp_trans->on_handle_rtcp(data, len, &payloadtype);

      if (isSR &&_network_observer) {
        _network_observer->OnReceivedRTCP((unsigned char*)buffer->data_ptr(), len, payloadtype);
      }
    }
  }

  int RTPDownloadInternal::SetLostRate(uint32_t send_lost_rate, uint32_t recv_lost_rate) {
    _send_lost_rate = send_lost_rate;
    _recv_lost_rate = recv_lost_rate;
    if (_channel.get()) {
      _channel->set_lost_rate(send_lost_rate, recv_lost_rate);
    }
    return 0;
  }

  void RTPDownloadInternal::GetNetworkState(DownloadNetworkState* statistic) {
    memcpy(statistic, &m_network_statistic, sizeof(m_network_statistic));
  }

  RTPDownloadInternal::~RTPDownloadInternal() {
    INF("~RTPDownloadSDKImpl %p", this);

    Stop();

    _rtp_trans_config.reset();
    _channel.reset();
    _rtp_media_cache.reset();
    delete _buffer;

    delete m_retry_task;
  }

  void RTPDownloadInternal::OnFecRecoverPacket(uint8_t* RTPPacket, uint16_t len, void *arg) {
    ((RTPDownloadInternal *)arg)->OnFecRecoverPacketImpl(RTPPacket, len);
  }

  void RTPDownloadInternal::OnFecRecoverPacketImpl(uint8_t *data, uint16_t len) {
    if (len > 0) {
      RTP_FIXED_HEADER* rtp = (RTP_FIXED_HEADER*)data;
      TRC("fec_recover, payload=%d, seqNum=%d", (int)rtp->payload, (int)rtp->get_seq());
      if (_network_observer) {
        _network_observer->OnReceivedRTP(data, len, rtp->payload);
      }
    }
  }

  void RTPDownloadInternal::SetNetworkChanged() {
    INF("newwork changed");
    bool running = IsThreadIdValid(&m_worker_thread);
    Stop();
    if (running) {
      Start(&m_user_config);
    }
  }

  RTPTransConfig* RTPDownloadInternal::GetRTPTransConfig() {
    return _rtp_trans_config.get();
  }

  void RTPDownloadInternal::EnableFec(bool enable) {
    if (enable) {
      _rtp_trans_config->fec_rtp_count = 4;
    }
    else {
      _rtp_trans_config->fec_rtp_count = 0;
    }
  }

  void RTPDownloadInternal::EnableNack(bool enable) {
    if (enable) {
      _rtp_trans_config->max_nacklst_size = 250;
    }
    else {
      _rtp_trans_config->max_nacklst_size = 0;
    }
  }

}
