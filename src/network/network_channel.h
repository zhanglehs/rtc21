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
#include "../util/base_exception.h"
#include <string>

#ifdef WIN32
#include <ws2tcpip.h>
#include <Windows.h>
#define socklen_t int 
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

namespace network
{

  class SetSockOptException : public live_stream_sdk::BaseException
  {
  public:
    SetSockOptException(int32_t socket, int32_t level, int32_t opt_name);
    ~SetSockOptException()  throw(){}
  };

  struct addrinfo * getaddr(const char * hostname,
    const char * portname,
    int sockettype);

  int network_init();

  class BaseNetworkChannel
  {
  public:
    enum ChannelState
    {
      Construct = 1,
      Connecting = 2,
      Connected = 3,
      Error = 4,
      Closed = 5,
    };

    virtual int flush() = 0;
    virtual int init(const char* remote_ip, uint16_t remote_port, bool nonblock) = 0;
    virtual int init(int socket, struct addrinfo * remote_addr) = 0;
    virtual int send_data(const char* data, uint32_t len) = 0;
    virtual int receive_data(char* data, uint32_t max_len) = 0;
    virtual int get_sock_fd() { return _socket; }
    virtual struct addrinfo * get_addrinfo() { return _addr; }
    virtual int close() = 0;
    virtual void set_nonblock();
    virtual int get_state();
    virtual ~BaseNetworkChannel(){};
    virtual int check_connected() = 0;

    virtual int set_lost_rate(uint32_t send_lost_rate, uint32_t recv_lost_rate);

  protected:
    int _socket;
    struct addrinfo *_addr;
    ChannelState _state;

  protected:
    uint32_t _send_lost_rate;
    uint32_t _recv_lost_rate;
  };

  bool errno_eagain();
  bool errno_einter();
  bool errno_enobufs();

  union ip_t
  {
    unsigned int ipi;
    struct ip_byte
    {
      unsigned char byte[4];
    } ipb;
  };

  const char* ip2str(unsigned int host_order_ip);
  const char* ip2str(sockaddr *addr);

}
