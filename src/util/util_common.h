/**
* @file
* @brief
* @author   songshenyi
* <pre><b>copyright: Youku</b></pre>
* <pre><b>email: </b>songshenyi@youku.com</pre>
* <pre><b>company: </b>http://www.youku.com</pre>
* <pre><b>All rights reserved.</b></pre>
* @date 2015/07/24
* @see
*/


#pragma once

#include <pthread.h>

#ifdef WIN32
#include <ws2tcpip.h>
#endif

namespace avutil
{
    char* util_ip2str_no_r(unsigned int network_order_ip, char *str, size_t sz);

    const char * util_ip2str_no(unsigned int network_order_ip);

    unsigned long long net_util_htonll(unsigned long long val);

    unsigned long long net_util_ntohll(unsigned long long val);

    int sprintfcat(char *buffer, const char *format, ...);

    void util_usleep(int microsecond);
    void util_msleep(unsigned int ms);

    int CheckAlias(const char* alias);

#ifdef WIN32
    int gettimeofday(struct timeval *tp, void *tzp);
#endif

	void dumpBacktrace();
}

namespace live_stream_sdk {

  bool IsThreadIdValid(pthread_t *threadid);
  bool IsAliasValid(const char* alias);

  class CLocalLock {
  public:
    CLocalLock(pthread_mutex_t *mutex) : m_mutex(mutex) {
      pthread_mutex_lock(m_mutex);
    }
    ~CLocalLock() {
      pthread_mutex_unlock(m_mutex);
    }
  private:
    pthread_mutex_t *m_mutex;
  };

}
