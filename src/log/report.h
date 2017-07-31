#pragma once

#include <pthread.h>
#include <event.h>

#ifndef WIN32
#ifndef PTW32_CDECL
#define PTW32_CDECL
#endif
#endif

namespace live_stream_sdk {

  class CLogReport {
  public:
    CLogReport();
    ~CLogReport();

    void HttpGet(const char *url);
    void HttpPost(const char *url, const void *data, int len);
    // TODO: zhangle
    void Quit();

  private:
    static void* PTW32_CDECL WorkerThread(void* arg);
    void* WorkerThreadImpl();
    static void OnTimer(evutil_socket_t, short, void *);
    void OnTimerImpl();
    pthread_t m_threadid;
    struct event_base *m_ev_base;
  };
}
