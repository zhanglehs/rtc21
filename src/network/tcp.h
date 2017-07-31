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

#include <stdint.h>

#include "network_channel.h"
#include "../util/data_buffer.h"

#ifdef WIN32

#include <ws2tcpip.h>
#include <Windows.h>

#else
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#endif


#if defined(WIN32)
#define snprintf _snprintf
#else

typedef int Socket;
#ifndef INVALID_SOCKET 
#define INVALID_SOCKET -1
#endif
#ifndef SOCKET_ERROR 
#define SOCKET_ERROR -1
#endif

#define closesocket(fd) ::close(fd)

#define WSANOTINITIALISED  EPROTONOSUPPORT

#endif

namespace network
{
  int tcp_init();

  class CMDTCPChannel : public BaseNetworkChannel
  {
  public:
    CMDTCPChannel();
    virtual int flush();
    virtual int init(const char* remote_ip, uint16_t remote_port, bool nonblock);
    virtual int init(int socket, struct addrinfo * addr);
    virtual int send_data(const char* data, uint32_t len);
    virtual int receive_data(char* data, uint32_t max_len);
    virtual int close();
    virtual int check_connected();
    virtual ~CMDTCPChannel();

  protected:


    live_stream_sdk::DataBuffer* _read_buffer;
    live_stream_sdk::DataBuffer* _write_buffer;
  };


}
