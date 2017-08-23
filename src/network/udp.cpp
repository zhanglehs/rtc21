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


#include "udp.h"
#include "../log/log.h"
#include <stdio.h>
#include "errno.h"
#include <ctime>

#ifdef WIN32

#else
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#endif

using namespace live_stream_sdk;

namespace network
{

  SimpleUDPChannel::SimpleUDPChannel()
  {
    _send_lost_rate = 0;
    _recv_lost_rate = 0;
    srand(unsigned(time(0)));
    _state = Construct;
  }

  int SimpleUDPChannel::flush()
  {
    return 0;
  }

  int SimpleUDPChannel::init(const char* remote_ip, uint16_t remote_port, bool nonblock)
  {
    struct addrinfo *addr;
    char _addr_s[20];
    sprintf(_addr_s, "%u", remote_port);

    addr = getaddr(remote_ip, _addr_s, SOCK_DGRAM);

    _socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

    if (_socket < 0)
    {
      ERR("create socket failed,_socket:%d, error: %s", _socket, strerror(errno));
      //throw CreateSocketException(NetworkProto::UDP, strerror(errno));
      return -1;
    }

    if (nonblock)
    {
      set_nonblock();
    }

    int ret = init(_socket, addr);

    return ret;
  }

  int SimpleUDPChannel::init(int socket, struct addrinfo* addr)
  {
    _socket = socket;
    _addr = addr;
    int n = 1024 * 1024;

    if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char*)&n, sizeof(n)) < 0)
    {
      ERR("init channel failed, error: %s", strerror(errno));
      throw SetSockOptException(socket, SOL_SOCKET, SO_SNDBUF);
    }

    n = 1024 * 1024;
    if (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (const char*)&n, sizeof(n)) < 0)
    {
      ERR("init channel failed, error: %s", strerror(errno));
      throw SetSockOptException(socket, SOL_SOCKET, SO_RCVBUF);
    }

    int ret = 0;

    ret = connect(_socket, _addr->ai_addr, _addr->ai_addrlen);
    if (ret < 0)
    {
      ERR("connect failed, error: %s", strerror(errno));
      //throw SocketConnectException(socket, ip2str(_addr->ai_addr),
      //  ntohs(*((uint16_t*)_addr->ai_addr->sa_data)), NetworkProto::UDP, strerror(errno));
      return ret;
    }

    _state = Connected;
    return ret;
  }

  int SimpleUDPChannel::send_data(const char* data, uint32_t len)
  {
    //int ret = sendto(this->_socket, data, len, 0, (const sockaddr*)(&_addr), sizeof(_addr));

    if (_send_lost_rate > 0)
    {
      uint32_t i = rand();
      if (i % 100 < _send_lost_rate)
      {
        return 0;
      }
    }

    int ret = sendto(_socket, data, len, 0, NULL, 0);

    if (ret < 0)
    {
      if (errno_eagain() || errno_einter() || errno_enobufs())
      {
        ret = 0;
        return ret;
      }

      _state = Error;
      //throw SocketReadWriteException(_socket, ip2str(_addr->ai_addr),
      //  ntohs(*((uint16_t*)_addr->ai_addr->sa_data)), NetworkProto::UDP, strerror(errno));
      return ret;
    }

    return ret;
  }

  int SimpleUDPChannel::receive_data(char* data, uint32_t max_len) {
    socklen_t addr_len = _addr->ai_addrlen;

    int ret = recvfrom(_socket, data, max_len, 0, _addr->ai_addr, &addr_len);
    if (ret < 0)  {
      if (errno_eagain() || errno_einter())  {
        return 0;
      }
      //throw SocketReadWriteException(_socket, ip2str(_addr->ai_addr),
      //  ntohs(*((uint16_t*)_addr->ai_addr->sa_data)), NetworkProto::UDP, strerror(errno));
      return -1;
    }

    if (_recv_lost_rate > 0) {
      uint32_t i = rand();
      if (i % 100 < _recv_lost_rate) {
        return 0;
      }
    }

    return ret;
  }

  int SimpleUDPChannel::send_data(DataBuffer* buf)
  {
    int ret = sendto(_socket, (char*)buf->data_ptr(), buf->data_len(), 0, _addr->ai_addr, _addr->ai_addrlen);

    return ret;
  }

  int SimpleUDPChannel::receive_data(DataBuffer* buf, uint32_t max_len)
  {
    char data[65536];

    socklen_t addr_len = _addr->ai_addrlen;

    int ret = recvfrom(_socket, data, max_len, 0, _addr->ai_addr, &addr_len);

    if (ret > 0)
    {
      buf->append_ptr(data, ret);
    }

    return ret;
  }


  int SimpleUDPChannel::close()
  {
    if (INVALID_SOCKET != _socket && (_state != Construct && _state != Closed))
    {
      shutdown(_socket, 2);
      closesocket(_socket);
      _socket = INVALID_SOCKET;
      _state = Closed;
    }
    return 0;
  }

  int SimpleUDPChannel::check_connected()
  {
    return 0;
  }

  SimpleUDPChannel::~SimpleUDPChannel()
  {
    INF("~SimpleUDPChannel");
    close();
    if (_addr != NULL)
    {
      freeaddrinfo(_addr);
    }
  }

  ServerUDPChannel::ServerUDPChannel()
  {
  }

  ServerUDPChannel::~ServerUDPChannel()
  {
    if (_addr != NULL)
    {
      freeaddrinfo(_addr);
    }
  }

  int ServerUDPChannel::init(int socket, struct addrinfo * addr)
  {
    _socket = socket;
    _addr = addr;
    int ret = 0;
    int n = 1024 * 1024;
    if (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (const char*)&n, sizeof(n)) == -1)
    {
      ret = -1;
      return ret;
    }

    if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char*)&n, sizeof(n)) == -1)
    {
      ret = -1;
      return ret;
    }

    ret = bind(_socket, _addr->ai_addr, _addr->ai_addrlen);

    if (ret < 0)
    {
#ifdef WIN32
      ERR("bind failed, error: %d", WSAGetLastError());
#else
      ERR("bind failed, error: %s", strerror(errno));
#endif
      //throw SocketConnectException(socket, ip2str(_addr->ai_addr),
      //  ntohs(*((uint16_t*)_addr->ai_addr->sa_data)), NetworkProto::UDP, strerror(errno));
      return ret;
    }
    return ret;
  }
}
