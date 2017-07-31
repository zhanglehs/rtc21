#include "../engine_api/rtp_api.h"
#include "../engine_api/rtp_download.h"
#include "../sdk/rtp_format.h"

#ifdef WIN32
#include <WinSock2.h>
#elif defined(__ANDROID__)
#include "../superlogic/RtcPlayerInternal.h"
#endif

lfrtcGlobalConfig* getAvengineParamsSDK() {
  static lfrtcGlobalConfig g_avengine_params_sdk;
  return &g_avengine_params_sdk;
}

#if defined(__ANDROID__)
int rtcAndroidInit(void* jvm, void* context) {
  RtcPlayerInternal::jvm_ = (JavaVM*)jvm;
  return IISetAndroidObj(jvm, context);
}

int rtcAndroidUninit() {
  return IIClearAndroidObj();
}
#endif


int rtcInit() {
#ifdef WIN32
  WSADATA wsaData;
  WORD wVersionRequested = MAKEWORD(2, 2);
  int32_t err = WSAStartup(wVersionRequested, &wsaData);
  if (err != 0) {
    return -1;
  }
#endif
  return 0;
}

void rtcUninit() {
}


//////////////////////////////////////////////////////////////////////////
#include "../network/CHttpFetch.h"
#include "../sdk/streamid.h"
#include <json/json.h>

namespace {
  void OnCreateStreamFinish(char *streamid, int httpcode, const char* data, int len) {
    if (httpcode >= 200 && httpcode < 300) {
      Json::Reader jreader;
      Json::Value jroot;
      jreader.parse(data, data + len, jroot, false);
      if (!jroot.isNull() && jroot.isObject()) {
        Json::Value jstreamid = jroot["stream_id"];
        if (jstreamid.isString()) {
          StreamId_Ext id;
          id.parse(jstreamid.asCString(), 10);
          strcpy(streamid, id.unparse().c_str());
        }
      }
    }
  }
}

void create_stream_sync(char *streamid, const char *url) {
  streamid[0] = 0;

  struct event_base *base = event_base_new();
  CHttpFetch *http = CHttpFetch::Create(base);
  http->Get(url, std::bind(OnCreateStreamFinish, streamid, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  event_base_dispatch(base);
  event_base_loopbreak(base);
  CHttpFetch::Destroy(http);
  event_base_free(base);
}

namespace {
  void OnHttpGetFinish(char *content, int httpcode, const char* data, int len) {
    if (httpcode >= 200 && httpcode < 300) {
      strcpy(content, data);
    }
  }
}

void http_get_sync(char *content, const char *url) {
  content[0] = 0;
  struct event_base *base = event_base_new();
  CHttpFetch *http = CHttpFetch::Create(base);
  http->Get(url, std::bind(OnHttpGetFinish, content, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
  event_base_dispatch(base);
  event_base_loopbreak(base);
  CHttpFetch::Destroy(http);
  event_base_free(base);
}
