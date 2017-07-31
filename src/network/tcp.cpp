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


#include <ctime>
#include "tcp.h"
#include "../cmd_protocol/proto_define.h"
#include "../log/log.h"
#include "../cmd_protocol/proto_common.h"

#include <errno.h>
#include <stdlib.h>

#ifdef WIN32
#include <ws2tcpip.h>
#include <Windows.h>
#else
#include <sys/select.h>
#include <unistd.h>
#include <string>
#endif

using namespace live_stream_sdk;

namespace network
{
  CMDTCPChannel::CMDTCPChannel()
  {
    _read_buffer = new DataBuffer(1024 * 1024);
    _write_buffer = new DataBuffer(1024 * 1024);
    _state = Construct;

    _send_lost_rate = 0;
    _recv_lost_rate = 0;
    srand(unsigned(time(0)));
  }

  int CMDTCPChannel::flush()
  {
    int ret = 0;

    if (_state == Connected)
    {
      ret = _write_buffer->write_tcp_fd(_socket);
    }

    return ret;
  }

  int CMDTCPChannel::init(const char* remote_ip, uint16_t remote_port, bool nonblock)
  {
    struct addrinfo* _addr;

    INF("create tcp socket, %s, %u", remote_ip, remote_port);

    char _addr_s[20];
    sprintf(_addr_s, "%u", remote_port);

    _addr = getaddr(remote_ip, _addr_s, SOCK_STREAM);

    _socket = socket(_addr->ai_family, _addr->ai_socktype, _addr->ai_protocol);

    if (_socket < 0)
    {
      ERR("create socket failed,_socket:%d, error: %s", _socket, strerror(errno));
      //throw CreateSocketException(NetworkProto::TCP, strerror(errno));
      return -1;
    }

    if (nonblock)
    {
      set_nonblock();
    }

    int ret = init(_socket, _addr);

    return ret;
  }

  int CMDTCPChannel::init(int socket, struct addrinfo * addr)
  {
    _socket = socket;
    _addr = addr;
    int n = 1024 * 1024;

    if (setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (const char*)&n, sizeof(n)) < 0)
    {
      ERR("set send buffer failed, error: %s", strerror(errno));
      throw SetSockOptException(socket, SOL_SOCKET, SO_SNDBUF);
    }

    n = 1024 * 1024;
    if (setsockopt(socket, SOL_SOCKET, SO_RCVBUF, (const char*)&n, sizeof(n)) < 0)
    {
      ERR("set receive buffer failed, error: %s", strerror(errno));
      throw SetSockOptException(socket, SOL_SOCKET, SO_RCVBUF);
    }

    int ret = 0;

    ret = connect(_socket, _addr->ai_addr, _addr->ai_addrlen);
    if (ret < 0 && errno != EINPROGRESS && errno != 0)
    {
#ifdef WIN32
      int win_error = WSAGetLastError();
      if (win_error != WSAEWOULDBLOCK)
      {
#endif
        ERR("connect failed, error: %s", strerror(errno));
        //throw SocketConnectException(socket, ip2str(_addr->ai_addr),
        //  ntohs(*((uint16_t*)_addr->ai_addr->sa_data + 1)), NetworkProto::TCP, strerror(errno));
        return ret;
#ifdef WIN32
      }
#endif
    }

    _state = Connecting;
    return ret;
  }

  int CMDTCPChannel::check_connected()
  {
    if (_state == Connected)
    {
      return 0;
    }

    if (_state != Connecting)
    {
      return -1;
    }

    fd_set write_fds;
    FD_ZERO(&write_fds);
    FD_SET(_socket, &write_fds);

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(_socket, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 10;

    int nready = select(_socket + 1, &read_fds, &write_fds, NULL, &timeout);

    if (nready <= 0)
    {
      return 1;
    }

    if (FD_ISSET(_socket, &read_fds) && FD_ISSET(_socket, &write_fds))
    {
      _state = Error;
      //throw SocketConnectException(_socket, ip2str(_addr->ai_addr),
      //  ntohs(*((uint16_t*)_addr->ai_addr->sa_data)), NetworkProto::TCP, "check_connected failed");
      return -1;
    }

    if (FD_ISSET(_socket, &write_fds))
    {
      _state = Connected;
      return 0;
    }

    return 1;
  }

  int CMDTCPChannel::send_data(const char* data, uint32_t len)
  {
    if (_send_lost_rate > 0)
    {
      uint32_t i = rand();
      if (i % 100 < _send_lost_rate)
      {
        return 0;
      }
    }

    int ret = check_connected();
    if (ret < 0)
    {
      return ret;
    }

    _write_buffer->append_ptr(data, len);

    ret = _write_buffer->write_tcp_fd(_socket);

    if (ret < 0)
    {
      _state = Error;
      //throw SocketReadWriteException(_socket, ip2str(_addr->ai_addr),
      //  ntohs(*((uint16_t*)_addr->ai_addr->sa_data)), NetworkProto::TCP, strerror(errno));
      return ret;
    }

    return ret;
  }

  int CMDTCPChannel::receive_data(char* data, uint32_t max_len)
  {
    if (_recv_lost_rate > 0)
    {
      uint32_t i = rand();
      if (i % 100 < _recv_lost_rate)
      {
        return 0;
      }
    }

    int ret = check_connected();
    if (ret < 0)
    {
      return ret;
    }

    ret = _read_buffer->read_tcp_fd(_socket);

    if (ret < 0)
    {
      //throw SocketReadWriteException(_socket, ip2str(_addr->ai_addr),
      //  ntohs(*((uint16_t*)_addr->ai_addr->sa_data)), NetworkProto::TCP, strerror(errno));
      return ret;
    }

    if (_read_buffer->data_len() < sizeof(proto_header))
    {
      return 0;
    }

    proto_header h;
    ret = decode_header(_read_buffer, &h);
    if (ret < 0)
    {
      return 0;
    }

    if (_read_buffer->data_len() < h.size)
    {
      return 0;
    }

    memcpy(data, _read_buffer->data_ptr(), h.size);

    _read_buffer->ignore(h.size);

    return h.size;
  }

  int CMDTCPChannel::close()
  {
    if (INVALID_SOCKET != _socket)
    {
      shutdown(_socket, 2);
      closesocket(_socket);
      _socket = INVALID_SOCKET;
    }

    return 0;
  }

  CMDTCPChannel::~CMDTCPChannel()
  {
    close();
    if (_read_buffer != NULL)
    {
      delete _read_buffer;
      _read_buffer = NULL;
    }

    if (_write_buffer != NULL)
    {
      delete _write_buffer;
      _write_buffer = NULL;
    }
    if (_addr != NULL)
    {
      freeaddrinfo(_addr);
    }
  }
}
