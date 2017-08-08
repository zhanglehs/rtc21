#include "CHttpFetch.h"

#include <event2/thread.h>
#include <assert.h>

#ifdef WIN32
#pragma comment(lib, "libevent.lib")
#pragma comment(lib, "libevent_core.lib")
#pragma comment(lib, "libevent_extras.lib")
//#pragma comment(lib, "libs/pthreadVC2d.lib")
#pragma comment(lib, "ws2_32.lib")
#else
#ifndef PTW32_CDECL
#define PTW32_CDECL
#endif
#endif

CEventTask::CEventTask() {
  m_self_delete = false;
  event_assign(&m_event, NULL, -1, 0, NULL, NULL);
}

CEventTask::~CEventTask() {
  Cancel();
}

int CEventTask::Post(struct event_base *base, std::function<void()> task, unsigned int delay_ms) {
  m_task = task;
  struct timeval tv;
  tv.tv_sec = delay_ms / 1000;
  tv.tv_usec = (delay_ms % 1000) * 1000;
  event_assign(&m_event, base, -1, 0, OnTask, (void*)this);
  return event_add(&m_event, &tv);
}

void CEventTask::Cancel() {
  if (event_get_base(&m_event)) {
    event_del(&m_event);
  }
  event_assign(&m_event, NULL, -1, 0, NULL, NULL);
}

int CEventTask::Post2(struct event_base *base, std::function<void()> task, unsigned int delay_ms) {
  CEventTask *ev_task = new CEventTask();
  ev_task->m_self_delete = true;
  int ret = ev_task->Post(base, task, delay_ms);
  if (ret < 0) {
    delete ev_task;
  }
  return ret;
}

const struct event* CEventTask::GetEvent() {
  return &m_event;
}

void CEventTask::OnTask(evutil_socket_t fd, short event, void *arg) {
  CEventTask *pThis = (CEventTask *)arg;
  pThis->OnTaskImpl();
}

void CEventTask::OnTaskImpl() {
  if (m_task) {
    m_task();
  }
  if (m_self_delete) {
    delete this;
  }
}

//////////////////////////////////////////////////////////////////////////

namespace {
  void * PTW32_CDECL CHttpFetchInternalThread(void *arg) {
    struct event_base *base = (struct event_base *)arg;
    event_base_dispatch(base);
    return NULL;
  }

  void CHttpFetchKeepThread(evutil_socket_t fd, short event, void *arg) {
    int i = 0;
    i++;
  }
}

struct event_base *CHttpFetch::m_thread_event_base = NULL;
pthread_mutex_t CHttpFetch::m_mutex = PTHREAD_MUTEX_INITIALIZER;
std::set<CHttpFetch*> CHttpFetch::m_self_delete_http;
std::set<CHttpFetch*> CHttpFetch::m_https;
bool CHttpFetch::m_thread_quit = false;
//struct event CHttpFetch::m_thread_event;
pthread_t *CHttpFetch::m_thread_id = NULL;

CHttpFetch::CHttpFetch(struct event_base *base, bool autofree) {
  pthread_mutex_lock(&m_mutex);
  m_https.insert(this);
  pthread_mutex_unlock(&m_mutex);
  m_base = base;
  m_ev_connection = NULL;
  m_ev_request = NULL;
  m_self_delete = autofree;
  //m_running_event = NULL;
  //m_callbacking = false;
//#ifdef WIN32
//  m_worker_threadid = 0;
//#else
//  TODO
//#endif
}

CHttpFetch::~CHttpFetch() {
  CancelInternal();
  if (!event_base_got_break(m_base) && !event_base_got_exit(m_base)
    && event_base_get_running_event(m_base) == m_done_task.GetEvent()) {
    assert(!"CHttpFetch::~CHttpFetch() is called in CHttpFetch's callback function, this is not allowed");
  }
  pthread_mutex_lock(&m_mutex);
  m_https.erase(this);
  pthread_mutex_unlock(&m_mutex);
}

CHttpFetch* CHttpFetch::Create(bool autofree /*= false*/) {
  if (m_thread_event_base == NULL) {
    // TODO: zhangle
#ifdef WIN32
    evthread_use_windows_threads();
#else
    evthread_use_pthreads();
#endif
    pthread_mutex_lock(&m_mutex);
    if (m_thread_event_base == NULL && !m_thread_quit) {
      m_thread_event_base = event_base_new();
      static struct event s_thread_event;
      struct timeval tv;
      evutil_timerclear(&tv);
      tv.tv_sec = 10000;
      event_assign(&s_thread_event, m_thread_event_base, -1, EV_PERSIST, CHttpFetchKeepThread, NULL);
      event_add(&s_thread_event, &tv);

      m_thread_id = new pthread_t;
      memset(m_thread_id, 0, sizeof(pthread_t));
      pthread_create(m_thread_id, NULL, CHttpFetchInternalThread, m_thread_event_base);
    }
    pthread_mutex_unlock(&m_mutex);
  }
  if (m_thread_event_base == NULL) {
    return NULL;
  }
  CHttpFetch *http = new CHttpFetch(m_thread_event_base, autofree);
  if (autofree) {
    pthread_mutex_lock(&m_mutex);
    m_self_delete_http.insert(http);
    pthread_mutex_unlock(&m_mutex);
  }
  return http;
}

CHttpFetch* CHttpFetch::Create(struct event_base *base) {
  return new CHttpFetch(base, false);
}

void CHttpFetch::Destroy(CHttpFetch* http) {
  if (http && http->m_self_delete) {
    assert(!"Autofree CHttpFetch should not deleted by user. Probably the user double delete a normal ChttpFetch pointer, or memory fault");
    return;
  }
  delete http;
}

void CHttpFetch::FinalDestroy() {
  pthread_mutex_lock(&m_mutex);
  m_thread_quit = true;
  pthread_mutex_unlock(&m_mutex);

  if (m_thread_id) {
    event_base_loopbreak(m_thread_event_base);
    //event_del(&m_thread_event);
    void *ret = NULL;
    pthread_join(*m_thread_id, &ret);
    delete m_thread_id;
    m_thread_id = NULL;
    event_base_free(m_thread_event_base);
    m_thread_event_base = NULL;
  }

  for (auto it = m_self_delete_http.begin(); it != m_self_delete_http.end(); it++) {
    delete *it;
  }
}

void CHttpFetch::Get(const char *url,
  const std::function<void(int, const char *, unsigned int)> &callback) {
  //m_callback = callback;

  //struct evhttp_uri *ev_uri = evhttp_uri_parse(url);
  //const char *host = evhttp_uri_get_host(ev_uri);
  //int port = evhttp_uri_get_port(ev_uri);
  //if (port <= 0) {
  //  port = 80;
  //}
  //const char *query = evhttp_uri_get_query(ev_uri);
  //const char *path = evhttp_uri_get_path(ev_uri);
  //std::string path_query = "/";
  //if (path) {
  //  path_query = path;
  //}
  //if (query) {
  //  path_query += '?';
  //  path_query += query;
  //}

  //// connection need freed manually. This function will copy the address string, and finally will be free automatically by libevent
  //struct evhttp_connection *ev_connection = evhttp_connection_base_new(m_base, NULL, host, port);
  //evhttp_connection_free_on_completion(ev_connection);
  //struct evhttp_request *ev_request = evhttp_request_new(OnHttpFinished, this);
  //// if not call evhttp_request_own, request will be freed automatically by libevent
  ////evhttp_request_own(ev_request);
  //m_request_header["Host"] = host;
  //for (auto it = m_request_header.begin(); it != m_request_header.end(); it++) {
  //  // This function will copy the key and value string, and finally will be free automatically by libevent
  //  evhttp_add_header(ev_request->output_headers, it->first.c_str(), it->second.c_str());
  //}
  //// This function will copy the uri string, and finally will be free automatically by libevent
  //evhttp_make_request(ev_connection, ev_request, EVHTTP_REQ_GET, path_query.c_str());

  //// 几个地方传入的const char *指针，在libevent内部会copy一份，并最终自动释放，即上层无需保障指针的内容一直有效

  //evhttp_uri_free(ev_uri);
  //m_ev_connection = ev_connection;
  //m_ev_request = ev_request;
  Request(EVHTTP_REQ_GET, url, NULL, 0, callback);
}

void CHttpFetch::Post(const char *url, const char *data, int len,
  const std::function<void(int, const char *, unsigned int)> &callback) {
  //m_callback = callback;
  //if (len < 0) {
  //  len = strlen(data);
  //}

  //struct evhttp_uri *ev_uri = evhttp_uri_parse(url);
  //const char *host = evhttp_uri_get_host(ev_uri);
  //int port = evhttp_uri_get_port(ev_uri);
  //if (port <= 0) {
  //  port = 80;
  //}
  //const char *query = evhttp_uri_get_query(ev_uri);
  //const char *path = evhttp_uri_get_path(ev_uri);
  //std::string path_query = "/";
  //if (path) {
  //  path_query = path;
  //}
  //if (query) {
  //  path_query += '?';
  //  path_query += query;
  //}

  //// connection need freed manually. This function will copy the address string, and finally will be free automatically by libevent
  //struct evhttp_connection *ev_connection = evhttp_connection_base_new(m_base, NULL, host, port);
  //evhttp_connection_free_on_completion(ev_connection);
  //struct evhttp_request *ev_request = evhttp_request_new(OnHttpFinished, this);
  //// if not call evhttp_request_own, request will be freed automatically by libevent
  ////evhttp_request_own(ev_request);
  //m_request_header["Host"] = host;
  //m_request_header["Content-Type"] = "application/x-www-form-urlencoded";
  //for (auto it = m_request_header.begin(); it != m_request_header.end(); it++) {
  //  // This function will copy the key and value string, and finally will be free automatically by libevent
  //  evhttp_add_header(ev_request->output_headers, it->first.c_str(), it->second.c_str());
  //}
  //evbuffer_add(ev_request->output_buffer, data, len);
  //// This function will copy the uri string, and finally will be free automatically by libevent
  //evhttp_make_request(ev_connection, ev_request, EVHTTP_REQ_POST, path_query.c_str());

  //// 几个地方传入的const char *指针，在libevent内部会copy一份，并最终自动释放，即上层无需保障指针的内容一直有效

  //evhttp_uri_free(ev_uri);
  //m_ev_connection = ev_connection;
  //m_ev_request = ev_request;
  Request(EVHTTP_REQ_POST, url, data, len, callback);
}

void CHttpFetch::Put(const char *url, const char *data, int len,
  const std::function<void(int, const char *, unsigned int)> &callback) {
  //m_callback = callback;
  //if (len < 0) {
  //  len = strlen(data);
  //}

  //struct evhttp_uri *ev_uri = evhttp_uri_parse(url);
  //const char *host = evhttp_uri_get_host(ev_uri);
  //int port = evhttp_uri_get_port(ev_uri);
  //if (port <= 0) {
  //  port = 80;
  //}
  //const char *query = evhttp_uri_get_query(ev_uri);
  //const char *path = evhttp_uri_get_path(ev_uri);
  //std::string path_query = "/";
  //if (path) {
  //  path_query = path;
  //}
  //if (query) {
  //  path_query += '?';
  //  path_query += query;
  //}

  //// connection need freed manually. This function will copy the address string, and finally will be free automatically by libevent
  //struct evhttp_connection *ev_connection = evhttp_connection_base_new(m_base, NULL, host, port);
  //evhttp_connection_free_on_completion(ev_connection);
  //struct evhttp_request *ev_request = evhttp_request_new(OnHttpFinished, this);
  //// if not call evhttp_request_own, request will be freed automatically by libevent
  ////evhttp_request_own(ev_request);
  //m_request_header["Host"] = host;
  //m_request_header["Content-Type"] = "application/x-www-form-urlencoded";
  //for (auto it = m_request_header.begin(); it != m_request_header.end(); it++) {
  //  // This function will copy the key and value string, and finally will be free automatically by libevent
  //  evhttp_add_header(ev_request->output_headers, it->first.c_str(), it->second.c_str());
  //}
  //evbuffer_add(ev_request->output_buffer, data, len);
  //// This function will copy the uri string, and finally will be free automatically by libevent
  //evhttp_make_request(ev_connection, ev_request, EVHTTP_REQ_PUT, path_query.c_str());

  //// 几个地方传入的const char *指针，在libevent内部会copy一份，并最终自动释放，即上层无需保障指针的内容一直有效

  //evhttp_uri_free(ev_uri);
  //m_ev_connection = ev_connection;
  //m_ev_request = ev_request;
  Request(EVHTTP_REQ_PUT, url, data, len, callback);
}

void CHttpFetch::Request(enum evhttp_cmd_type type,
  const char *url, const char *data, int len,
  const std::function<void(int, const char *, unsigned int)> &callback) {

  m_callback = callback;
  if (len < 0) {
    len = (int)strlen(data);
  }

  struct evhttp_uri *ev_uri = evhttp_uri_parse(url);
  const char *host = evhttp_uri_get_host(ev_uri);
  int port = evhttp_uri_get_port(ev_uri);
  if (port <= 0) {
    port = 80;
  }
  const char *query = evhttp_uri_get_query(ev_uri);
  const char *path = evhttp_uri_get_path(ev_uri);
  std::string path_query = "/";
  if (path) {
    path_query = path;
  }
  if (query) {
    path_query += '?';
    path_query += query;
  }

  // connection need freed manually. This function will copy the address string, and finally will be free automatically by libevent
  struct evhttp_connection *ev_connection = evhttp_connection_base_new(m_base, NULL, host, port);
  evhttp_connection_free_on_completion(ev_connection);
  struct evhttp_request *ev_request = NULL;
  if (callback) {
    ev_request = evhttp_request_new(OnHttpFinished, this);
  }
  else {
    // TODO: zhangle, need check
    ev_request = evhttp_request_new(NULL, this);
  }
  // if not call evhttp_request_own, request will be freed automatically by libevent
  //evhttp_request_own(ev_request);
  m_request_header["Host"] = host;
  m_request_header["Content-Type"] = "application/x-www-form-urlencoded";
  for (auto it = m_request_header.begin(); it != m_request_header.end(); it++) {
    // This function will copy the key and value string, and finally will be free automatically by libevent
    evhttp_add_header(ev_request->output_headers, it->first.c_str(), it->second.c_str());
  }
  if (data && len > 0) {
    evbuffer_add(ev_request->output_buffer, data, len);
  }
  // This function will copy the uri string, and finally will be free automatically by libevent
  evhttp_make_request(ev_connection, ev_request, type, path_query.c_str());

  // 几个地方传入的const char *指针，在libevent内部会copy一份，并最终自动释放，即上层无需保障指针的内容一直有效

  evhttp_uri_free(ev_uri);
  m_ev_connection = ev_connection;
  m_ev_request = ev_request;
}

void CHttpFetch::CancelInternal() {
  if (!event_base_got_break(m_base) && !event_base_got_exit(m_base)
    && event_base_get_running_event(m_base) == m_done_task.GetEvent()) {
    // 此时表示http回调中调用了本方法。这是很不友好的调用方式，应尽量避免该使用方式
    return;
  }

  if (m_callback) {
    pthread_mutex_lock(&m_mutex);
    m_callback = std::function<void(int, const char *, unsigned int)>();
    pthread_mutex_unlock(&m_mutex);
  }
  m_done_task.Cancel();
}

void CHttpFetch::AddRequestHeader(const char *key, const char *value) {
  m_request_header[key] = value;
}

void CHttpFetch::OnHttpFinished(struct evhttp_request *req, void *arg){
  CHttpFetch *pThis = (CHttpFetch *)arg;
  pthread_mutex_lock(&m_mutex);
  auto it = m_https.find(pThis);
  if (it != m_https.end()) {
    pThis->OnHttpFinishedImpl(req);
  }
  pthread_mutex_unlock(&m_mutex);
}

void CHttpFetch::OnHttpFinishedImpl(struct evhttp_request *req) {
  //m_running_event = event_base_get_running_event(m_base);
//#ifdef WIN32
//  m_worker_threadid = GetCurrentThreadId();
//#else
//  TODO
//#endif

  int response_code = 0;
  if (req) {
    response_code = req->response_code;
  }
  const char *content = NULL;
  unsigned int len = 0;
  if ((response_code >= 200 && response_code < 300) || (response_code >= 400 && response_code < 500)) {
    struct evkeyval *header_node = req->input_headers->tqh_first;
    while (header_node) {
      m_response_header[header_node->key] = header_node->value;
      header_node = header_node->next.tqe_next;
    }

    struct evbuffer* buf = evhttp_request_get_input_buffer(req);
    len = (unsigned int)evbuffer_get_length(buf);
    //evbuffer_expand(buf, 1);
    content = (const char *)evbuffer_pullup(buf, -1);
  }
  else if (response_code == HTTP_MOVEPERM) {
    // HTTP_MOVETEMP
  }

  m_response_content.resize(len + 1);
  if (len > 0) {
    memcpy(&m_response_content[0], content, len);
  }
  m_response_content[len] = 0;

  if (m_callback) {
    m_done_task.Post(m_base, std::bind(m_callback, response_code, &m_response_content[0], len));
    m_callback = std::function<void(int, const char *, unsigned int)>();
  }

  //if (m_callback) {
  //  m_callbacking = true;
  //  m_callback(response_code, content, len);
  //  m_callbacking = false;
  //  m_callback = std::function<void(int, const char *, unsigned int)>();
  //}

  if (m_self_delete) {
    CEventTask::Post2(m_base, std::bind(&CHttpFetch::OnAutoFree, this));
  }
}

void CHttpFetch::OnAutoFree(CHttpFetch *http) {
  pthread_mutex_lock(&m_mutex);
  m_self_delete_http.erase(http);
  pthread_mutex_unlock(&m_mutex);
  delete http;
}

void CHttpFetch::Cancel() {
  if (!m_self_delete) {
    CancelInternal();
  }
}
