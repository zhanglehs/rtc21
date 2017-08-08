#include "rtp_upload_internal.h"

#include "../avformat/rtcp.h"
#include "../cmd_protocol/proto_common.h"
#include "../cmd_protocol/proto_rtp_rtcp.h"
#include "../log/log.h"
#include "../media_manager/rtp_block_cache.h"
#include "../network/tcp.h"
#include "../network/udp.h"
#include "../util/util_common.h"
#include "../upload/rtp_package.h"
#include "../network/CHttpFetch.h"
#include "../rtp_trans/rtp_sender_trans.h"
#include "../sdk/rtp_format.h"
#include "../sdk/sdk_interface.h"
#include <json/json.h>

#ifndef WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#else
#include <ws2tcpip.h>
#include <Windows.h>
#endif

#include <stdio.h>
#include <string>
#include <memory>

// TODO: zhangle
#include <event2/thread.h>

namespace live_stream_sdk {

  RTPUploadInternal::RTPUploadInternal(RtpNetworkObserver* observer) {
    // TODO: zhangle
#ifdef WIN32
    evthread_use_windows_threads();
#else
    evthread_use_pthreads();
#endif

    INF("RTPUploadSDKImpl construct %p", this);

    _network_observer = observer;
    _rtp_trans_config.reset(new RTPTransConfig);

    _send_data_buffer.reset(new DataBuffer(256 * 256));
    _recv_data_buffer.reset(new DataBuffer(256 * 256));

    _estimate_bitrate = 0;

    _channel.reset();

    //INF("_upload_thread create RTPUploadInternal");
    srand((unsigned int)time(NULL));

    _video_expect_bitrate = 1000000;

    _send_lost_rate = 0;
    _recv_lost_rate = 0;

    _rtp_package = NULL;
    _rtp_trans = NULL;
    memset(&m_worker_thread, 0, sizeof(m_worker_thread));
    m_ev_base = NULL;
    m_ev_writable = NULL;
    m_ev_readable = NULL;
    m_ev_closed = NULL;
    m_ev_trans_timer = NULL;
    m_ev_send = NULL;
    m_http_upload_url = NULL;
    m_http_mcu = NULL;
    m_http_sdp = NULL;
    m_retry_task = new CEventTask();
    m_net_writable = false;
    m_quit_thread = false;
    m_udp_available = false;
    m_udp_connect_count = 0;
    m_audio_ssrc = 0;
    m_video_ssrc = 0;
    pthread_mutex_init(&m_mutex, NULL);
    memset(&m_network_statistic, 0, sizeof(m_network_statistic));
    m_audio_frequence = 48000;
#ifdef WIN32
    // TODO: zhangle, 这个值应从codec中获取
    m_audio_frame_size = 1024;
#else
    m_audio_frame_size = 1024;
#endif
    m_state = RtcCaptureState::RTC_CAPTURE_STATE_UNSET;
    m_state_internal = RtcCaptureState::RTC_CAPTURE_STATE_UNSET;
  }

  RTPUploadInternal::~RTPUploadInternal() {
    INF("~RTPUploadSDKImpl %p", this);

    Stop();

    _send_data_buffer.reset();
    _recv_data_buffer.reset();

    _rtp_trans_config.reset();
    _rtp_media_cache.reset();
    _channel.reset();

    pthread_mutex_destroy(&m_mutex);
  }

  int32_t RTPUploadInternal::Start(const rtcUploadDispatchConfig* config) {
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
    memset(&m_network_statistic, 0, sizeof(m_network_statistic));

    m_audio_ssrc = 0;
    m_video_ssrc = 0;
    srand((unsigned int)time(NULL) + (unsigned int)clock());
    while (m_audio_ssrc == 0) {
      m_audio_ssrc = rand();
    }
    while (m_video_ssrc == 0) {
      m_video_ssrc = rand();
    }
    rtp_media_info h264_media_info;
    h264_media_info.rate = 90000;
    h264_media_info.payload_type = RTP_AV_H264;
    h264_media_info.rtp_transport = RTP_AVP;
    rtp_media_info aac_media_info;
    aac_media_info.rate = m_audio_frequence;
    aac_media_info.h264_profile_level_id = 2;
    aac_media_info.channels = 2;
    aac_media_info.payload_type = RTP_AV_AAC;
    aac_media_info.rtp_transport = RTP_AVP;
    aac_media_info.constant_duration = m_audio_frame_size;
    std::vector<rtp_media_info*> media_infos;
    media_infos.push_back(&h264_media_info);
    media_infos.push_back(&aac_media_info);
    SdpInfo sdp_parser;
    sdp_parser.set_sdp(media_infos);
    m_sdp = sdp_parser.get_sdp_str();

    memcpy(&m_user_config, config, sizeof(m_user_config));
    memcpy(&m_internal_config, config, sizeof(m_internal_config));

    SetState(RtcCaptureState::RTC_CAPTURE_STATE_INITIALIZING);
    if (pthread_create(&m_worker_thread, NULL, WorkerThread, this) < 0) {
      if (_network_observer) {
        _network_observer->OnNetworkState(RtcCaptureState::RTC_CAPTURE_STATE_ERROR, RtcCaptureErrorType::RTC_CAPTURE_ERROR_CREATETHREAD_FAILED);
        _network_observer->OnNetworkState(RtcCaptureState::RTC_CAPTURE_STATE_STOPPED);
      }
      return -1;
    }
    return 0;
  }

  void * PTW32_CDECL RTPUploadInternal::WorkerThread(void *arg) {
    RTPUploadInternal *pThis = (RTPUploadInternal*)arg;
    return pThis->WorkerThreadImpl();
  }

  void* RTPUploadInternal::WorkerThreadImpl() {
    m_ev_base = event_base_new();

    m_ev_send = event_new(m_ev_base, -1, EV_TIMEOUT | EV_PERSIST, OnSendData, this);
    event_add(m_ev_send, NULL);

    _rtp_package = new rtcRTPPackage(m_audio_ssrc, m_video_ssrc,
      m_audio_frequence, m_audio_frame_size);

    while (!m_quit_thread) {
      GetUploadUrl();
      event_base_dispatch(m_ev_base);
      event_base_loopbreak(m_ev_base);
      ClearWorkerThread();
    }

    if (m_ev_send) {
      CLocalLock lock(&m_mutex);
      event_free(m_ev_send);
      m_ev_send = NULL;
    }

    if (_rtp_package) {
      delete _rtp_package;
      _rtp_package = NULL;
    }

    return NULL;
  }

  void RTPUploadInternal::ClearWorkerThread() {
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

    if (m_http_upload_url) {
      CHttpFetch::Destroy(m_http_upload_url);
      m_http_upload_url = NULL;
    }
    if (m_http_mcu) {
      CHttpFetch::Destroy(m_http_mcu);
      m_http_mcu = NULL;
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

  void RTPUploadInternal::OnSocketWritable(evutil_socket_t fd, short flag, void *arg) {
    RTPUploadInternal *pThis = (RTPUploadInternal*)arg;
    pThis->OnSocketWritableImpl(fd, flag);
  }

  void RTPUploadInternal::OnSocketReadable(evutil_socket_t fd, short flag, void *arg) {
    RTPUploadInternal *pThis = (RTPUploadInternal*)arg;
    pThis->OnSocketReadableImpl(fd, flag);
  }

  void RTPUploadInternal::OnSocketClosed(evutil_socket_t fd, short flag, void *arg) {
    RTPUploadInternal *pThis = (RTPUploadInternal*)arg;
    pThis->OnSocketClosedImpl(fd, flag);
  }

  void RTPUploadInternal::OnTransTimer(evutil_socket_t fd, short flag, void *arg) {
    RTPUploadInternal *pThis = (RTPUploadInternal*)arg;
    pThis->OnTransTimerImpl(fd, flag);
  }

  void RTPUploadInternal::OnSendData(evutil_socket_t fd, short flag, void *arg) {
    RTPUploadInternal *pThis = (RTPUploadInternal*)arg;
    pThis->OnSendDataImpl(fd, flag);
  }

  void RTPUploadInternal::OnSocketWritableImpl(evutil_socket_t, short) {
    m_net_writable = true;
    if (m_state_internal == InternalState::INTERNAL_STATE_MCU_CONNECTING) {
      _send_upload_request();
      m_ev_readable = event_new(m_ev_base, _channel->get_sock_fd(), EV_READ | EV_PERSIST, OnSocketReadable, this);
      event_add(m_ev_readable, NULL);
      m_ev_trans_timer = event_new(m_ev_base, -1, EV_PERSIST, OnTransTimer, this);
      struct timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;
      event_add(m_ev_trans_timer, &tv);
    }
    if (m_state == RtcCaptureState::RTC_CAPTURE_STATE_RUNNING) {
      SendPacket();
    }
  }

  void RTPUploadInternal::OnSocketReadableImpl(evutil_socket_t, short) {
    if (m_state_internal == InternalState::INTERNAL_STATE_MCU_ACKING) {
      _receive_upload_ack();
    }
    if (m_state == RtcCaptureState::RTC_CAPTURE_STATE_RUNNING) {
      _receive_rtcp();
    }
  }

  void RTPUploadInternal::OnSocketClosedImpl(evutil_socket_t, short) {
    if (m_state == RtcCaptureState::RTC_CAPTURE_STATE_RUNNING) {
      SetState(RtcCaptureState::RTC_CAPTURE_STATE_ERROR, RtcCaptureErrorType::RTC_CAPTURE_ERROR_NETWORK_ERROR);
    }
    m_retry_task->Post(m_ev_base, std::bind(&RTPUploadInternal::Retry, this), 1000);
  }

  void RTPUploadInternal::OnTransTimerImpl(evutil_socket_t, short) {
    _rtp_trans->on_timer();
    if (!_rtp_trans->is_alive()) {
      if (m_state == RtcCaptureState::RTC_CAPTURE_STATE_RUNNING) {
        SetState(RtcCaptureState::RTC_CAPTURE_STATE_ERROR, RtcCaptureErrorType::RTC_CAPTURE_ERROR_NETWORK_TIMEOUT);
      }
      Retry();
    }
    else {
      m_network_statistic.audio_total_send_packet_count = _rtp_trans->get_audio_total_rtp_packetcount();
      m_network_statistic.audio_total_lost_packet_count = _rtp_trans->get_audio_total_lost_packet_count();
      m_network_statistic.audio_bps = _rtp_trans->get_audio_current_bitrate();
      m_network_statistic.audio_total_bytes = (unsigned int)_rtp_trans->get_audio_total_rtp_bytes(); // TODO: zhangle
      m_network_statistic.video_total_send_packet_count = _rtp_trans->get_video_total_rtp_packetcount();
      m_network_statistic.video_total_lost_packet_count = _rtp_trans->get_video_frac_packet_lost_rate();
      m_network_statistic.video_bps = _rtp_trans->get_video_current_bitrate();
      m_network_statistic.video_total_bytes = (unsigned int)_rtp_trans->get_video_total_rtp_bytes(); // TODO: zhangle
      m_network_statistic.packet_lost_percent = _rtp_trans->get_frac_packet_lost_rate() * 100 / 255;
      m_network_statistic.rtt_ms = _rtp_trans->get_rtt_ms();
      m_network_statistic.is_tcp = m_internal_config.is_tcp;
    }
  }

  void RTPUploadInternal::OnSendDataImpl(evutil_socket_t, short) {
    if (m_state_internal != RtcCaptureState::RTC_CAPTURE_STATE_RUNNING) {
      return;
    }
    SendPacket();
  }

  void RTPUploadInternal::Retry() {
    event_base_loopbreak(m_ev_base);
  }

  void RTPUploadInternal::SetState(RtcCaptureState state, int reason) {
    m_state_internal = state;
    m_state = state;
    if (_network_observer) {
      _network_observer->OnNetworkState(m_state, reason);
    }
  }

  void RTPUploadInternal::GetUploadUrl() {
    memcpy(&m_internal_config, &m_user_config, sizeof(m_internal_config));
    m_state_internal = InternalState::INTERNAL_STATE_GET_UPLOAD_URL;
    m_http_upload_url = CHttpFetch::Create(m_ev_base);
    char url[1024];
    memset(url, 0, sizeof(url));
    sprintf(url, "http://%s/v1/get_upload_url?app_id=%s&alias=%s&upload_token=%s",
      m_user_config.lapi_host,
      m_user_config.appid, m_user_config.alias, m_user_config.lapi_token);
    m_http_upload_url->Get(url,
      std::bind(&RTPUploadInternal::OnGetUploadUrlFinish, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  }

  void RTPUploadInternal::OnGetUploadUrlFinish(int code, const char* data, int len) {
    bool success = false;
    if (code >= 200 && code < 300) {
      if (data && len > 0) {
        std::string url;
        unsigned int stream_id = 0;

        try {
          Json::Reader reader;
          Json::Value jroot;
          reader.parse(data, data + len, jroot, false);
          if (jroot.isObject() && !jroot.isNull()) {
            Json::Value jcode = jroot["error_code"];
            Json::Value jurl = jroot["upload_url"];
            Json::Value jid = jroot["stream_id"];

            if (jurl.isString()) {
              url = jurl.asString();
            }

            if (jid.isNumeric()) {
              stream_id = jid.asUInt();
            }
            success = true;
          }
          else {
            ERR("parse get upload url response failed, response: %s", data);
          }
        }
        catch (std::exception &) {
          ERR("parse get upload url response failed, response: %s", data);
        }

        if (success) {
          if (m_internal_config.upload_url[0] == 0) {
            strcpy(m_internal_config.upload_url, url.c_str());
          }
          if (m_internal_config.streamid[0] == 0) {
            StreamId_Ext id;
            id = stream_id;
            strcpy(m_internal_config.streamid, id.unparse().c_str());
          }
          if (m_internal_config.upload_url[0]) {
            OnGetUploadUrlSuccessed();
            return;
          }
        }
      }
    }
    else {
      if (data && len > 0) {
        ERR("get upload url failed, response: %s", data);
      }
    }

    OnGetUploadUrlFailed(code);
  }

  void RTPUploadInternal::OnGetUploadUrlSuccessed() {
    GetMcuInfo();
  }

  void RTPUploadInternal::OnGetUploadUrlFailed(int httpcode) {
    if (httpcode >= 200 && httpcode < 600) {
      m_quit_thread = true;
      event_base_loopbreak(m_ev_base);
      SetState(RtcCaptureState::RTC_CAPTURE_STATE_ERROR, RtcCaptureErrorType::RTC_CAPTURE_ERROR_GETUPLOADURL_FAILED);
      SetState(RtcCaptureState::RTC_CAPTURE_STATE_STOPPED, RtcCaptureStopType::RTC_CAPTURE_STOP_BY_ERROR);
    }
    else {
      m_retry_task->Post(m_ev_base, std::bind(&RTPUploadInternal::Retry, this), 1000);
    }
  }

  void RTPUploadInternal::GetMcuInfo() {
    m_state_internal = InternalState::INTERNAL_STATE_GET_MCU_INFO;
    if (m_user_config.mcu_udp_port > 0 && m_user_config.sdp_url[0]) {
      OnGetMcuInfoSuccessed();
      return;
    }
    m_http_mcu = CHttpFetch::Create(m_ev_base);
    m_http_mcu->Get(m_internal_config.upload_url,
      std::bind(&RTPUploadInternal::OnGetMcuInfoFinish, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  }

  void RTPUploadInternal::OnGetMcuInfoFinish(int code, const char* data, int len) {
    bool success = false;
    if (code >= 200 && code < 300) {
      if (data && len > 0) {

        std::string ip;
        unsigned int udp_port = 0;
        unsigned int tcp_port = 0;
        std::string token;
        unsigned int stream_id = 0;
        std::string sdp_url;

        try {
          Json::Reader jreader;
          Json::Value jroot;
          jreader.parse(data, data + len, jroot, false);
          if (jroot.isObject() && !jroot.isNull()) {
            Json::Value jparams = jroot["params"];
            if (jparams.isObject() && !jparams.isNull()) {
              Json::Value jip = jparams["uploader_ip"];
              Json::Value judp = jparams["rtp_udp_port"];
              Json::Value jtcp = jparams["rtp_tcp_port"];
              Json::Value jtoken = jparams["token"];
              Json::Value jid = jparams["stream_id"];
              Json::Value jurl = jparams["sdp_upload_url"];

              if (jip.isString()) {
                ip = jip.asString();
              }
              if (judp.isNumeric()) {
                udp_port = judp.asUInt();
              }
              if (jtcp.isNumeric()) {
                tcp_port = jtcp.asUInt();
              }
              if (jtoken.isString()) {
                token = jtoken.asString();
              }
              if (jid.isNumeric()) {
                stream_id = jid.asUInt();
              }
              if (jurl.isString()) {
                sdp_url = jurl.asString();
              }
              success = true;
            }
          }
          else {
            ERR("parse get lus response failed, response: %s", data);
          }
        }
        catch (std::exception &) {
          ERR("parse get lus response failed, response: %s", data);
        }

        if (success) {
          if (m_internal_config.mcu_ip[0] == 0) {
            strcpy(m_internal_config.mcu_ip, ip.c_str());
          }
          if (m_internal_config.mcu_udp_port == 0) {
            m_internal_config.mcu_udp_port = (uint16_t)udp_port;
          }
          if (m_internal_config.mcu_tcp_port == 0) {
            m_internal_config.mcu_tcp_port = (uint16_t)tcp_port;
          }
          if (m_internal_config.mcu_token[0] == 0) {
            strcpy(m_internal_config.mcu_token, token.c_str());
          }
          if (m_internal_config.sdp_url[0] == 0) {
            strcpy(m_internal_config.sdp_url, sdp_url.c_str());
          }

          if (m_internal_config.sdp_url[0]) {
            OnGetMcuInfoSuccessed();
            return;
          }
        }
      }
    }
    else {
      if (data && len > 0) {
        ERR("get lus failed, response: %s", data);
      }
    }

    OnGetMcuInfoFailed(code);
  }

  void RTPUploadInternal::OnGetMcuInfoSuccessed() {
    SendSdp();
  }

  void RTPUploadInternal::OnGetMcuInfoFailed(int httpcode) {
    if (httpcode >= 200 && httpcode < 600) {
      m_quit_thread = true;
      event_base_loopbreak(m_ev_base);
      SetState(RtcCaptureState::RTC_CAPTURE_STATE_ERROR, RtcCaptureErrorType::RTC_CAPTURE_ERROR_GETMCUINFO_FAILED);
      SetState(RtcCaptureState::RTC_CAPTURE_STATE_STOPPED, RtcCaptureStopType::RTC_CAPTURE_STOP_BY_ERROR);
    }
    else {
      m_retry_task->Post(m_ev_base, std::bind(&RTPUploadInternal::Retry, this), 1000);
    }
  }

  void RTPUploadInternal::SendSdp() {
    m_state_internal = InternalState::INTERNAL_STATE_SEND_SDP;
    m_http_sdp = CHttpFetch::Create(m_ev_base);
    m_http_sdp->Put(m_internal_config.sdp_url, m_sdp.c_str(), -1,
      std::bind(&RTPUploadInternal::OnSendSdpFinish, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  }

  void RTPUploadInternal::OnSendSdpFinish(int httpcode, const char* data, int len) {
    if (httpcode >= 200 && httpcode < 300) {
      OnSendSdpSuccessed();
    }
    else {
      OnSendSdpFailed(httpcode);
    }
  }

  void RTPUploadInternal::OnSendSdpSuccessed() {
    StreamId_Ext streamid;
    streamid.parse(m_internal_config.streamid);
    _rtp_media_cache.reset(new media_manager::RTPMediaCache(streamid));
    _rtp_media_cache->set_sdp(m_sdp);

    _rtp_trans = new RTPSendTrans(streamid, _rtp_trans_config);
    _rtp_trans->set_rtp_cache(_rtp_media_cache.get());
    _rtp_trans->set_video_expect_bitrate(_video_expect_bitrate);

    _begin_connect();
  }

  void RTPUploadInternal::OnSendSdpFailed(int httpcode) {
    if (httpcode >= 200 && httpcode < 600) {
      m_quit_thread = true;
      event_base_loopbreak(m_ev_base);
      SetState(RtcCaptureState::RTC_CAPTURE_STATE_ERROR, RtcCaptureErrorType::RTC_CAPTURE_ERROR_SENDSDP_FAILED);
      SetState(RtcCaptureState::RTC_CAPTURE_STATE_STOPPED, RtcCaptureStopType::RTC_CAPTURE_STOP_BY_ERROR);
    }
    else {
      m_retry_task->Post(m_ev_base, std::bind(&RTPUploadInternal::Retry, this), 1000);
    }
  }

  int32_t RTPUploadInternal::_begin_connect() {
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

    try {
      if (tcp) {
        _channel.reset(new network::CMDTCPChannel());
        INF("upload server ip:%s,tcp port:%d.", m_internal_config.mcu_ip, (int)m_internal_config.mcu_tcp_port);
        _channel->init(m_internal_config.mcu_ip, m_internal_config.mcu_tcp_port, true);
      }
      else {
        m_udp_connect_count++;
        _channel.reset(new network::SimpleUDPChannel());
        INF("upload server ip:%s,udp port:%d.", m_internal_config.mcu_ip, (int)m_internal_config.mcu_udp_port);
        _channel->init(m_internal_config.mcu_ip, m_internal_config.mcu_udp_port, true);
      }

      _channel->set_lost_rate(_send_lost_rate, _recv_lost_rate);
      _channel->set_nonblock();
      _rtp_trans->set_channel(_channel);

      INF("setup channel success, waiting for connected.");
      m_state_internal = InternalState::INTERNAL_STATE_MCU_CONNECTING;
    }
    catch (BaseException&) {
      ERR("setup channel failed, reconnecting...");
      m_retry_task->Post(m_ev_base, std::bind(&RTPUploadInternal::Retry, this), 1000);
      return -1;
    }

    m_ev_closed = event_new(m_ev_base, _channel->get_sock_fd(), EV_CLOSED | EV_PERSIST, OnSocketClosed, this);
    m_ev_writable = event_new(m_ev_base, _channel->get_sock_fd(), EV_WRITE, OnSocketWritable, this);
    event_add(m_ev_closed, NULL);
    event_add(m_ev_writable, NULL);

    return 0;
  }

  int32_t RTPUploadInternal::_send_upload_request() {
    DataBuffer buf(256 * 256);
    buf.clear();

    rtp_u2r_req_state req;
    memset(&req, 0, sizeof(req));
    req.payload_type = PAYLOAD_TYPE_RTP;
    StreamId_Ext sid;
    sid.parse(m_internal_config.streamid);
    memcpy(req.streamid, &sid, sizeof(sid));
    memcpy(req.token, m_internal_config.mcu_token, strlen(m_internal_config.mcu_token));
    req.version = 1;
    req.user_id = 2;

    encode_rtp_u2r_req_state_ext(&req, GetLiveSDKDeviceID(), &buf);
    _channel->send_data((char*)buf.data_ptr(), static_cast<uint32_t>(buf.data_len()));

    //INF("send upload request, sid:%s", _stream_info._stream_id.unparse().c_str());
    m_state_internal = InternalState::INTERNAL_STATE_MCU_ACKING;
    return 0;
  }

  int32_t RTPUploadInternal::_receive_upload_ack() {
    uint8_t buffer[256 * 256];
    int len = _channel->receive_data((char*)buffer, sizeof(buffer));
    if (len <= 0) {
      return -1;
    }

    DataBuffer buf(sizeof(buffer));
    buf.clear();
    buf.append_ptr(buffer, len);

    StreamId_Ext stream_id;
    rtp_u2r_rsp_state rsp;
    int ret = decode_rtp_u2r_rsp_state(&rsp, &buf);
    if (ret >= 0) {
      memcpy(&stream_id, rsp.streamid, sizeof(StreamId_Ext));
      if (stream_id.unparse() != m_internal_config.streamid) {
        WRN("upload sdk streamid not match");
        //sprintf(msg, "upload sdk streamid not match recv %d send %d", stream_id.get_32bit_stream_id(), _stream_info._stream_id.get_32bit_stream_id());
        return -1;
      }
      if (rsp.result != 0) {
        m_quit_thread = true;
        event_base_loopbreak(m_ev_base);
        SetState(RtcCaptureState::RTC_CAPTURE_STATE_ERROR, RtcCaptureErrorType::RTC_CAPTURE_ERROR_MCU_DENY);
        SetState(RtcCaptureState::RTC_CAPTURE_STATE_STOPPED, RtcCaptureStopType::RTC_CAPTURE_STOP_BY_ERROR);
        return -1;
      }
    }
    else {
      INF("not a udp_rsp_pkt, try decode as rtcp ret %d", ret);
      ret = decode_rtcp_u2r_packet(stream_id, &buf);
      if (ret >= 0) {
        if (stream_id.unparse() != m_internal_config.streamid) {
          WRN("upload sdk streamid not match");
          //sprintf(msg, "upload sdk streamid not match recv %d send %d", stream_id.get_32bit_stream_id(), _stream_info._stream_id.get_32bit_stream_id());
          return -1;
        }
      }
      else {
        return -1;
      }
    }

    SetState(RtcCaptureState::RTC_CAPTURE_STATE_RUNNING);
    SendPacket();
    return 0;
  }

  int32_t RTPUploadInternal::_receive_rtcp() {
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
      _recv_data_buffer->clear();
      _recv_data_buffer->append_ptr(buffer, len);
      _on_recv_packet(_recv_data_buffer.get());
    }
    return 0;
  }

  // TODO: zhangle, multi thread
  int RTPUploadInternal::SetLostRate(uint32_t send_lost_rate, uint32_t recv_lost_rate) {
    _send_lost_rate = send_lost_rate;
    _recv_lost_rate = recv_lost_rate;

    if (_channel.get()) {
      _channel->set_lost_rate(send_lost_rate, recv_lost_rate);
    }

    return 0;
  }

  void RTPUploadInternal::GetSSRC(uint32_t &audio, uint32_t &video) {
    audio = m_audio_ssrc;
    video = m_video_ssrc;
  }

  void RTPUploadInternal::EnableFec(bool enable) {
    if (enable) {
      _rtp_trans_config->fec_rtp_count = 4;
    }
    else {
      _rtp_trans_config->fec_rtp_count = 0;
    }
  }

  void RTPUploadInternal::EnableNack(bool enable) {
    if (enable) {
      _rtp_trans_config->max_nacklst_size = 250;
    }
    else {
      _rtp_trans_config->max_nacklst_size = 0;
    }
  }

  void RTPUploadInternal::GetDispatchConfig(rtcUploadDispatchConfig *config) {
    memcpy(config, &m_internal_config, sizeof(m_internal_config));
  }

  void RTPUploadInternal::GetNetworkState(UploadNetworkState* statistic) {
    memcpy(statistic, &m_network_statistic, sizeof(m_network_statistic));
  }

  void RTPUploadInternal::SendPacket() {
    EncodedFrame frame;
    while (m_net_writable) {
      {
        CLocalLock lock(&m_mutex);
        if (m_packets.empty()) {
          return;
        }
        frame = m_packets.front();
        m_packets.pop_front();
      }

      if (frame.video) {
        std::vector<rtcRTPPackage::RtpPacket> packets;
        _rtp_package->PackH264(packets, frame.buf, (int)frame.len, frame.width, frame.height, frame.ts_ms);
        for (auto it = packets.begin(); it != packets.end(); it++) {
          SendRTP((const uint8_t*)it->buf, it->len);
        }
      }
      else {
        rtcRTPPackage::RtpPacket packet;
        if (_rtp_package->PackAAC(&packet, frame.buf, (int)frame.len, frame.ts_ms) >= 0) {
          SendRTP((const uint8_t*)packet.buf, packet.len);
        }
      }

      delete[] frame.buf;
    }
  }

  int32_t RTPUploadInternal::SendRTP(const uint8_t* RTPPacket, uint16_t len) {
    const avformat::RTP_FIXED_HEADER* rtp = (const avformat::RTP_FIXED_HEADER*)RTPPacket;
    RTPAVType type = (RTPAVType)rtp->payload;

    bool drop_packet = false;
    if (type == RTP_AV_H264) {
      drop_packet = _rtp_trans->drop_video_rtp_packet();
    }

    if (!drop_packet) {
      int32_t status;
      _rtp_media_cache->set_rtp(rtp, len, status);

      _send_data_buffer->clear();
      StreamId_Ext streamid;
      streamid.parse(m_internal_config.streamid);
      int ret = encode_rtp_u2r_packet(streamid, RTPPacket, len, _send_data_buffer.get());
      if (ret < 0) {
        // TODO: zhangle
        ERR("encode_rtp_u2r_packet error. ret: %d", ret);
        //throw RTPUploadEncodePacketException("encode_rtp_u2r_packet failed");
        return -1;
      }

      int send_count = _channel->send_data((char*)_send_data_buffer->data_ptr(), static_cast<uint32_t>(_send_data_buffer->data_len()));
      if (send_count == 0) {
        m_net_writable = false;
        event_add(m_ev_writable, NULL);
      }

      //TRACERTP("RTP_S_NORMAL streamid %s ssrc %u seq %d payload %d timestamp %u len %d", _stream_info._stream_id.unparse().c_str(), rtp->get_ssrc(), rtp->get_seq(), rtp->payload, rtp->get_rtp_timestamp(), len);

      _rtp_trans->on_handle_rtp(RTPPacket, len);//create session and send fec packet
    }
    else {
      //TRACERTP("RTP_S_NORMAL_DROP streamid %s ssrc %u seq %d payload %d timestamp %u len %d", _stream_info._stream_id.unparse().c_str(), rtp->get_ssrc(), rtp->get_seq(), rtp->payload, rtp->get_rtp_timestamp(), len);
    }
    return 0;
  }

  void RTPUploadInternal::_on_recv_packet(DataBuffer* buffer) {
    uint16_t payload_len;
    proto_header h;
    int ret = decode_header(buffer, &h);
    StreamId_Ext stream_id;

    switch (h.cmd) {
    case CMD_RTCP_U2R_PACKET:
      ret = decode_rtcp_u2r_packet(stream_id, buffer);//private protocal header 24 bytes
      if (ret < 0) {
        ERR("decode_rtcp_u2r_packet error, ret: %d", ret);
        break;
      }

      if (stream_id.unparse() == m_internal_config.streamid) {
        const uint8_t* payload = (const uint8_t*)buffer->data_ptr();
        payload_len = buffer->data_len();
        int payloadtype = 0;
        _rtp_trans->on_handle_rtcp(payload, payload_len, &payloadtype);
        if (_network_observer) {
          _network_observer->OnReceivedRTCP(payload, payload_len, payloadtype);
        }
      }
      else {
        //WRN("upload sdk streamid not mactch recv %d send %d", stream_id.get_32bit_stream_id(), _stream_info._stream_id.get_32bit_stream_id());
      }
      break;
    default:
      WRN("unrecognized packet, cmd: %u", h.cmd);
      break;
    }
  }

  void RTPUploadInternal::_on_recv_rtp_packet(DataBuffer* buffer) {
    if (buffer->data_len() == 0) {
      return;
    }
    _rtp_trans->on_handle_rtp((const uint8_t*)buffer->data_ptr(), static_cast<uint32_t>(buffer->data_len()));
  }

  int32_t RTPUploadInternal::Stop() {
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

    {
      CLocalLock lock(&m_mutex);
      for (auto it = m_packets.begin(); it != m_packets.end(); it++) {
        delete it->buf;
      }
      m_packets.clear();
    }

    if (m_state != RtcCaptureState::RTC_CAPTURE_STATE_UNSET && m_state != RtcCaptureState::RTC_CAPTURE_STATE_STOPPED) {
      m_state_internal = RtcCaptureState::RTC_CAPTURE_STATE_STOPPED;
      m_state = RtcCaptureState::RTC_CAPTURE_STATE_STOPPED;
    }
    return 0;
  }

  RTPTransConfig* RTPUploadInternal::GetRTPTransConfig() {
    return _rtp_trans_config.get();
  }

  void RTPUploadInternal::SetAudioParam(unsigned int frequence, unsigned int frame_size) {
    m_audio_frame_size = frame_size;
    m_audio_frequence = frequence;
  }

  int32_t RTPUploadInternal::SendAAC(char *data, int len, unsigned int timestamp_ms) {
    if (m_quit_thread || data == NULL || len <= 0) {
      return 0;
    }

    char *data2 = new char[len];
    memcpy(data2, data, len);

    CLocalLock lock(&m_mutex);
    if (m_packets.empty() && m_ev_send) {
      event_active(m_ev_send, EV_TIMEOUT, 1);
    }
    m_packets.push_back(EncodedFrame(data2, (unsigned int)len, timestamp_ms, true, false));
    while (m_packets.back().ts_ms - m_packets.front().ts_ms > 1500) {
      // drop packet, drop packet until a video keyframe
      // 思路：先找到1个非IDR的video，然后再找到1个IDR的video，此时可以丢包到IDR video。
      bool found_no_keyframe = false;
      bool dropped = false;
      for (auto it = m_packets.begin(); it != m_packets.end(); it++) {
        if (!it->video) {
          continue;
        }
        if (!it->keyframe) {
          found_no_keyframe = true;
        }
        else if (found_no_keyframe) {
          for (auto it2 = m_packets.begin(); it2 != it; it2++) {
            delete[] it2->buf;
          }
          m_packets.erase(m_packets.begin(), it);
          dropped = true;
          break;
        }
      }
      if (!dropped || m_packets.empty()) {
        break;
      }
    }
    return 0;
  }

  int32_t RTPUploadInternal::SendH264(char *data, int len, int width, int height, unsigned int timestamp_ms) {
    if (m_quit_thread || data == NULL || len < 4 || data[0] || data[1]) {
      return 0;
    }

    int nal_type = 0;
    if (data[2] == 1) {
      nal_type = data[3] & 0x1f;
    }
    else if (data[2] == 0 && len > 4 && data[3] == 1) {
      nal_type = data[4] & 0x1f;
    }
    if (nal_type == 0) {
      return 0;
    }

    bool keyframe = (nal_type == 5 || nal_type == 7 || nal_type == 8);
    char *data2 = new char[len];
    memcpy(data2, data, len);

    CLocalLock lock(&m_mutex);
    if (m_packets.empty() && m_ev_send) {
      event_active(m_ev_send, EV_TIMEOUT, 1);
    }
    m_packets.push_back(EncodedFrame(data2, (unsigned int)len, timestamp_ms, keyframe, true, width, height));
    return 0;
  }

  void RTPUploadInternal::GetVideoEstimate(uint32_t* bitrate, uint8_t* loss, int64_t* rtt) {
    if (_rtp_trans) {
      _rtp_trans->get_video_estimate(bitrate, loss, rtt);
      _estimate_bitrate = *bitrate;
    }
    else {
      _estimate_bitrate = 0;
      *bitrate = 0;
      *loss = 0;
      *rtt = 0;
      TRC("RTPUploadInternal::GetVideoEstimate, no rtp trans");
    }
  }

  void RTPUploadInternal::SetVideoExpectBitrate(uint32_t bitrate) {
    if (_rtp_trans) {
      _rtp_trans->set_video_expect_bitrate(bitrate);
    }
    else {
      _video_expect_bitrate = bitrate;
    }
  }

  RtcCaptureState RTPUploadInternal::GetState() {
    return m_state;
  }

  int32_t RTPUploadInternal::SetNetworkChanged() {
    INF("newwork changed");
    bool running = IsThreadIdValid(&m_worker_thread);
    Stop();
    if (running) {
      Start(&m_user_config);
    }
    return 0;
  }

}
