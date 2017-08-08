#include <pthread.h>

#include <string>
#include <map>
#include <vector>
#include <set>
#include <functional>

#include <event.h>
#include <evhttp.h>

class CEventTask {
public:
  CEventTask();
  ~CEventTask();

  int Post(struct event_base *base, std::function<void()> task, unsigned int delay_ms = 0);
  void Cancel();
  static int Post2(struct event_base *base, std::function<void()> task, unsigned int delay_ms = 0);

  const struct event* GetEvent();

private:
  static void OnTask(evutil_socket_t fd, short event, void *arg);
  void OnTaskImpl();
  struct event m_event;
  std::function<void()> m_task;
  bool m_self_delete;
};

class CHttpFetch {
public:
  // 不支持由外部指定base的autofree http
  static CHttpFetch* Create(bool autofree = false);
  static CHttpFetch* Create(struct event_base *base);
  static void Destroy(CHttpFetch* http);

  // 退出app，卸载so/dll时，调用该函数
  // （如果Create时某个CHttpFetch的base = NULL或autofree = true，则在退出模块时需清理）
  static void FinalDestroy();

  void Get(const char *url, const std::function<void(int, const char *, unsigned int)> &callback = std::function<void(int, const char *, unsigned int)>());
  void Post(const char *url, const char *data, int len, const std::function<void(int, const char *, unsigned int)> &callback = std::function<void(int, const char *, unsigned int)>());
  void Put(const char *url, const char *data, int len, const std::function<void(int, const char *, unsigned int)> &callback = std::function<void(int, const char *, unsigned int)>());
  void Cancel();
  void AddRequestHeader(const char *key, const char *value);

private:
  CHttpFetch(struct event_base *base, bool autofree);
  ~CHttpFetch();
  static void OnHttpFinished(struct evhttp_request *req, void *arg);
  void OnHttpFinishedImpl(struct evhttp_request *req);
  static void OnAutoFree(CHttpFetch *http);
  void CancelInternal();
  void Request(enum evhttp_cmd_type type, const char *url, const char *data, int len, const std::function<void(int, const char *, unsigned int)> &callback);

  struct event_base *m_base;
  std::function<void(int, const char *, unsigned int)> m_callback;
  struct evhttp_connection *m_ev_connection;
  struct evhttp_request *m_ev_request;
  std::map<std::string, std::string> m_request_header;
  std::map<std::string, std::string> m_response_header;
  std::vector<char> m_response_content;
  CEventTask m_done_task;
  bool m_self_delete;
  //struct event *m_running_event;
//  bool m_callbacking;
//#ifdef WIN32
//  DWORD m_worker_threadid;
//#else
//  TODO
//#endif

  static struct event_base *m_thread_event_base;
  static pthread_mutex_t m_mutex;
  static std::set<CHttpFetch*> m_self_delete_http;
  static std::set<CHttpFetch*> m_https;
  static bool m_thread_quit;
  //static struct event m_thread_event;
  static pthread_t *m_thread_id;
};
