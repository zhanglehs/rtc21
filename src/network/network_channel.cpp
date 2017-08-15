#include <stdio.h>
#include "network_channel.h"

#ifndef WIN32
#include <fcntl.h>
#include <errno.h>
#endif
#include <string.h>

using namespace live_stream_sdk;
using namespace std;

namespace network
{

  SetSockOptException::SetSockOptException(int32_t socket, int32_t level, int32_t opt_name)
    :BaseException("SetSockOptException")
  {
    char msg_buf[1024];
    sprintf(msg_buf, "setsockopt failed, socket: %d, level: %d, opt_name: %d",
      socket, level, opt_name);
    SetMessage(msg_buf);
  }

  struct addrinfo * getaddr(const char * hostname,
    const char * portname,
    int sockettype) {
    struct addrinfo hint = { 0 }, *addr = NULL;

    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = sockettype;

    int ret = getaddrinfo(hostname, portname, &hint, &addr);

    if (ret) {
      //ERR("Failed to resolve hostname %s: %s\n",
      //  hostname, gai_strerror(ret));
      //throw CreateSocketException((sockettype = SOCK_DGRAM) ? NetworkProto::UDP : NetworkProto::TCP, "getaddr error");
      return NULL;
    }

    return addr;
  }

  static bool network_initialized = false;

  int network_init()
  {
    if (network_initialized)
    {
      return 0;
    }

#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(1, 1);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
      return -1;
    }

    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
    {
      WSACleanup();
      return -1;
    }
#endif    
    network_initialized = true;
    return 0;
  }

  BaseNetworkChannel::BaseNetworkChannel() {
    _send_lost_rate = 0;
    _recv_lost_rate = 0;
    _state = Construct;
    _addr = NULL;
    _socket = -1;
  }

  void BaseNetworkChannel::set_nonblock()
  {
#ifdef WIN32                        
    u_long mode = 1;
    ioctlsocket(_socket, FIONBIO, &mode);
#else
    fcntl(_socket, F_SETFL, O_NONBLOCK);
#endif
  }

  int BaseNetworkChannel::get_state()
  {
    return _state;
  }

  int BaseNetworkChannel::set_lost_rate(uint32_t send_lost_rate, uint32_t recv_lost_rate)
  {
    _send_lost_rate = send_lost_rate;
    _recv_lost_rate = recv_lost_rate;
    return 0;
  }

  bool errno_eagain()
  {
    int error_no = 0;
#ifdef WIN32
    error_no = WSAGetLastError();
    if (error_no == WSAEWOULDBLOCK)
    {
      return true;
    }
#else
    error_no = errno;
    if (error_no == EAGAIN)
    {
      return true;
    }
#endif
    return false;
  }

  bool errno_einter()
  {
    int error_no = 0;
#ifdef WIN32
    error_no = WSAGetLastError();
    if (error_no == WSAEINTR)
    {
      return true;
    }
#else
    error_no = errno;
    if (error_no == EINTR)
    {
      return true;
    }
#endif
    return false;
  }

  bool errno_enobufs()
  {
    int error_no = 0;
#ifdef WIN32
    error_no = WSAGetLastError();
    if (error_no == WSAENOBUFS)
    {
      return true;
    }
#else
    error_no = errno;
    if (error_no == ENOBUFS)
    {
      return true;
    }
#endif
    return false;
  }

  char* ip2str_r(unsigned int host_order_ip, char *str, size_t sz)
  {
    union ip_t my_ip;

    my_ip.ipi = host_order_ip;

    memset(str, 0, sz);
    sprintf(str, "%u.%u.%u.%u",
      (unsigned int)my_ip.ipb.byte[3],
      (unsigned int)my_ip.ipb.byte[2],
      (unsigned int)my_ip.ipb.byte[1],
      (unsigned int)my_ip.ipb.byte[0]);
    return str;
  }

  char* ip2str_r(uint8_t *ip, char *str, size_t sz)
  {
    memset(str, 0, sz);
    sprintf(str, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
      *ip, *ip + 1, *ip + 2, *ip + 3,
      *ip + 4, *ip + 5, *ip + 6, *ip + 7,
      *ip + 8, *ip + 9, *ip + 10, *ip + 11,
      *ip + 12, *ip + 13, *ip + 14, *ip + 15);
    return str;
  }

  const char* ip2str(unsigned int host_order_ip)
  {
    static char buf[128];
    memset(buf, 0, sizeof(buf));

    ip2str_r(host_order_ip, buf, sizeof(buf));
    return buf;
  }

  const char* ip2str(sockaddr *addr)
  {
    static char buf[128];
    memset(buf, 0, sizeof(buf));
    if (addr->sa_family == AF_INET)
      ip2str_r(((sockaddr_in *)addr)->sin_addr.s_addr, buf, sizeof(buf));
    else
      ip2str_r(((sockaddr_in6 *)addr)->sin6_addr.s6_addr, buf, sizeof(buf));
    return buf;
  }
}
