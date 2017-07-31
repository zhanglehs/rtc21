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
#include <fcntl.h>
#include <sys/socket.h>


#ifndef INVALID_SOCKET 
#define INVALID_SOCKET -1
#endif
#ifndef SOCKET_ERROR 
#define SOCKET_ERROR -1
#endif

#define closesocket(fd) ::close(fd)

#define WSANOTINITIALISED  EPROTONOSUPPORT
#endif





#define MAX_UDP_PKT_SIZE 2048

namespace network
{
  class SimpleUDPChannel : public BaseNetworkChannel
  {
  public:
    SimpleUDPChannel();
    virtual int flush();
    virtual int init(const char* remote_ip, uint16_t remote_port, bool nonblock);
    virtual int init(int socket, struct addrinfo * addr);
    virtual int send_data(const char* data, uint32_t len);
    virtual int receive_data(char* data, uint32_t max_len);
    virtual int send_data(live_stream_sdk::DataBuffer* buf);
    virtual int receive_data(live_stream_sdk::DataBuffer* buf, uint32_t max_len);
    virtual int close();
    virtual ~SimpleUDPChannel();
    virtual int check_connected();
  };

  class ServerUDPChannel : public SimpleUDPChannel
  {
  public:
    ServerUDPChannel();
    virtual int init(int socket, struct addrinfo * addr);
    virtual ~ServerUDPChannel();
  };


}
