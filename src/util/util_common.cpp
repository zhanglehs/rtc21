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


#include "util_common.h"
#include <string>
#include <string.h>
#include <stdio.h>


#ifndef WIN32
#define _snprintf snprintf
#endif

#ifdef WIN32
#include <ws2tcpip.h>
#include <Windows.h>
#include <time.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#endif


#ifdef ANDROID
#include <iomanip>

#include <unwind.h>
#include <dlfcn.h> 
#endif
#include "../log/log.h"

using namespace live_stream_sdk;

namespace avutil
{
    union ip_t
    {
        unsigned int ipi;
        struct ip_byte
        {
            unsigned char byte[4];
        } ipb;
    };

    char* util_ip2str_no_r(unsigned int network_order_ip, char *str, size_t sz)
    {
        union ip_t my_ip;

        my_ip.ipi = network_order_ip;

        memset(str, 0, sz);
        _snprintf(str, sz, "%u.%u.%u.%u",
            (unsigned int)my_ip.ipb.byte[0],
            (unsigned int)my_ip.ipb.byte[1],
            (unsigned int)my_ip.ipb.byte[2],
            (unsigned int)my_ip.ipb.byte[3]);
        return str;
    }

    const char * util_ip2str_no(unsigned int network_order_ip)
    {
        static char buf[128];

        util_ip2str_no_r(network_order_ip, buf, sizeof(buf));
        return buf;
    }

    unsigned long long net_util_htonll(unsigned long long val)
    {
        return (((unsigned long long) htonl((int)((val << 32) >> 32))) <<
            32) | (unsigned int)htonl((int)(val >> 32));
    }

    unsigned long long net_util_ntohll(unsigned long long val)
    {
        return (((unsigned long long) htonl((int)((val << 32) >> 32))) <<
            32) | (unsigned int)htonl((int)(val >> 32));
    }
    int sprintfcat(char *buffer,const char *format,...) {
        int offset = strlen(buffer);
        if (offset < 0)
        {
            offset = 0;
        }
        va_list argp;
        va_start(argp, format);
#ifdef WIN32
        vsprintf(buffer + offset, format, argp);
#else
        vsprintf(buffer + offset, format, argp);
#endif
        va_end(argp);
        return strlen(buffer);
    }

    void util_usleep(int microsecond)
    {
#ifdef WIN32
        if (microsecond >= 1000)
        {
            Sleep(microsecond/1000);
        }
        else
        {
            LARGE_INTEGER freq;
            LARGE_INTEGER start, end;
            QueryPerformanceFrequency(&freq);
            LONGLONG count = (microsecond * freq.QuadPart) / (1000 * 1000);
            QueryPerformanceCounter(&start);
            count = count + start.QuadPart;
            do
            {
                QueryPerformanceCounter(&end);
            } while (end.QuadPart < count);
        }
#else
        usleep(microsecond);
#endif
    }

    void util_msleep(unsigned int ms) {
#ifdef WIN32
      Sleep(ms);
#else
      usleep(ms*1000);
#endif
    }

#ifdef WIN32
    int gettimeofday(struct timeval* tp, void *tzp)
    {
        time_t clock;
        struct tm tm;
        SYSTEMTIME wtm;
        GetLocalTime(&wtm);
        tm.tm_year = wtm.wYear - 1900;
        tm.tm_mon = wtm.wMonth - 1;
        tm.tm_mday = wtm.wDay;
        tm.tm_hour = wtm.wHour;
        tm.tm_min = wtm.wMinute;
        tm.tm_sec = wtm.wSecond;
        tm.tm_isdst = -1;
        clock = mktime(&tm);
        tp->tv_sec = (long)clock;
        tp->tv_usec = wtm.wMilliseconds * 1000;
        return (0);
    }
#endif

#ifdef ANDROID
	struct BacktraceState
	{
		void** current;
		void** end;
	};

	static _Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg)
	{
		BacktraceState* state = static_cast<BacktraceState*>(arg);
		uintptr_t pc = _Unwind_GetIP(context);
		if (pc) {
			if (state->current == state->end) {
				return _URC_END_OF_STACK;
			}
			else {
				*state->current++ = reinterpret_cast<void*>(pc);
			}
		}
		return _URC_NO_REASON;
	}

	size_t captureBacktrace(void** buffer, size_t max)
	{
		BacktraceState state = { buffer, buffer + max };
		_Unwind_Backtrace(unwindCallback, &state);

		return state.current - buffer;
	}
#endif

	void dumpBacktrace()
	{
#ifdef ANDROID
		size_t count = 50;
		void* buffer[count];

		count = captureBacktrace(buffer, count);

		for (size_t idx = 0; idx < count; ++idx) {
			const void* addr = buffer[idx];
			const char* symbol = "";

			Dl_info info;
			if (dladdr(addr, &info) && info.dli_sname) {
				symbol = info.dli_sname;
			}

			INF("  #%02d:%08x %s", idx, addr, symbol);
		}
#endif
	}

  int CheckAlias(const char* alias)
  {
    for (int i = 0; alias[i] != '\0'; i++)
    {
      if (alias[i] < '0'|| alias[i] > '9')
      {
        if (alias[i] < 'a'||alias[i] > 'z')
        {
          if (alias[i] < 'A'||alias[i] > 'Z')
          {
            if (alias[i] != '_')
            {
              return -1;
            }
          }
        }
      }
    }
    return 0;
  }

}

namespace live_stream_sdk {
  bool IsThreadIdValid(pthread_t *threadid) {
    // 只要指向的内存不为全0，则是有效的
    char *p = (char *)threadid;
    for (int i = 0; i < sizeof(pthread_t); i++) {
      if (p[i]) {
        return true;
      }
    }
    return false;
  }

  bool IsAliasValid(const char* alias) {
    if (!alias || !alias[0]) {
      return false;
    }

    char c;
    while (c = *(alias++)) {
      if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')) {
        return false;
      }
    }
    return true;
  }
}
