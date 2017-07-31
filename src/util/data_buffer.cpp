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



#include "data_buffer.h"
#include "../network/network_channel.h"
#include "../log/log.h"
#include <assert.h>
#include <stdlib.h>


#ifdef WIN32
#define NOMINMAX 
#include <ws2tcpip.h>
#include <Windows.h>
#define ioctl(x,y,z) ioctlsocket(x,y,(unsigned long *)(z))
#else
#include <netinet/in.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

#endif

#include <errno.h>
#include <algorithm>


using namespace network;

namespace live_stream_sdk
{
  DataBuffer::DataBuffer(uint32_t cap)
  {
    if (pthread_mutex_init(&m_mutex_write_buffer, NULL) != 0) {
      ERR("m_mutex_write_buffer fail");
      ///fail
    }

    if (pthread_mutex_init(&m_mutex_read_buffer, NULL) != 0) {
      ERR("m_mutex_read_buffer fail");
      ///fail
    }

    _start = 0;
    _end = 0;
    _size = cap;
    _max = cap;
#define BUFFER_PIECE_SIZE 64
    _size += BUFFER_PIECE_SIZE - (_size % BUFFER_PIECE_SIZE);
#undef BUFFER_PIECE_SIZE
    // there is no need to check _ptr is not a null
    _ptr = new char[_size];
  }

  DataBuffer::~DataBuffer()
  {
    if (_ptr)
    {
      delete _ptr;
      _ptr = 0;
    }
  }

  int DataBuffer::append(const DataBuffer * src)
  {
    assert(NULL != src);
    assert(NULL != src->_ptr);
    return append_ptr(src->data_ptr(), src->data_len());
  }

  int DataBuffer::append(const DataBuffer *src, size_t src_len)
  {
    size_t len = src_len < src->data_len() ?
    src_len : src->data_len();
    if (len > free_size())
    {
      return 0;
    }

    return append_ptr(src->data_ptr(), len);
  }

  int DataBuffer::append_ptr(const void *src, size_t src_len)
  {
    assert(NULL != src);

    if (free_size() < src_len)
    {
      return 0;
    }

    if (capacity() < src_len)
    {
      adjust();
    }

    memmove(_ptr + _end, src, src_len);
    _end += src_len;
    return static_cast<int>(src_len);
  }

  int DataBuffer::append_byte(unsigned char byte)
  {
    return append_ptr(&byte, 1);
  }

  int DataBuffer::write_buffer(DataBuffer *dst)
  {
    return dst->append(this);
  }

  int DataBuffer::write_buffer(void *dst, size_t dst_len)
  {
    size_t len = dst_len < data_len() ?
    dst_len : data_len();

    memmove(dst, data_ptr(), len);
    _start += len;
    return static_cast<int>(len);
  }

  void DataBuffer::try_adjust()
  {
    if (_start == _end)
    {
      _start = _end = 0;
      return;
    }
    if (_size >= 128 * 1024 || _start >= _size) {
      adjust();
    }
  }

  void DataBuffer::adjust()
  {
    memmove(_ptr, _ptr + _start, _end - _start);
    _end = _end - _start;
    _start = 0;
  }

  int DataBuffer::copy(const DataBuffer * src)
  {
    assert(NULL != src);
    assert(NULL != src->_ptr);
    if (src->data_len() == 0)
      return 0;

    if (src->data_len() > capacity())
      return -1;

    memmove(_ptr, src->data_ptr(), src->data_len());
    _end = src->data_len();
    return static_cast<int>(data_len());
  }

  int DataBuffer::eat(size_t bytes)
  {
    int len = (std::min)((int)bytes, (int)(_end - _start));
    _start += len;

    return len;
  }

  size_t DataBuffer::ignore(size_t bytes)
  {
    if (bytes == 0)
      return bytes;

    if (bytes >= _end - _start)
    {
      size_t ret = _end - _start;
      _start = _end = 0;
      return ret;
    }

    memmove(_ptr, _ptr + _start + bytes, _end - _start - bytes);
    _end -= _start + bytes;
    _start = 0;
    return bytes;
  }

  int DataBuffer::write_tcp_fd(int fd)
  {
    return write_tcp_fd_max(fd, 1024 * 1024);
  }

  int DataBuffer::write_tcp_fd_max(int fd, size_t max_len)
  {
    pthread_mutex_lock(&m_mutex_write_buffer);
    int ret = 0, total = 0;
    size_t end = (std::min)((uint32_t)(_start + max_len), (uint32_t)(_end));
    while (end - _start > 0)
    {
      ret = send(fd, _ptr + _start, end - _start, 0);

      if (ret < 0)
      {
        if (errno_eagain() || errno_einter())
        {
          break;
        }
        pthread_mutex_unlock(&m_mutex_write_buffer);
        return BUFFER_SOCKET_ERROR;
      }
      else if (0 == ret)
      {
        break;
      }
      else
      {
        total += ret;
        _start += ret;
      }
    }
    pthread_mutex_unlock(&m_mutex_write_buffer);
    return total;
  }

  int DataBuffer::read_tcp_fd(int fd)
  {
    return read_tcp_fd_max(fd, 1024 * 1024);
  }

  int DataBuffer::read_tcp_fd_max(int fd, size_t max_len)
  {
    int toread = 0, r;

    if (fd <= 0)
    {
      ERR("error fd or buffer null");
      return BUFFER_SOCKET_ERROR;
    }

    if (ioctl(fd, FIONREAD, &toread))
    {
      if (!errno_eagain() && !errno_einter())
      {
        ERR("ioctl error");
        return BUFFER_SOCKET_ERROR;
      }
      else
        return 0;
    }

    /* try to detect whether it's eof */
    if (toread == 0)
      toread = 1;

    toread = (std::min)((int)max_len, toread);
    toread = (std::min)((int)(_max - data_len()), toread);
    toread = (std::min)((int)capacity(), toread);

    r = recv(fd, _ptr + _end, toread, 0);
    if ((r == -1 && !errno_eagain() && !errno_einter()) || r == 0)
    {
      /* connection closed or reset */
      if (r == 0)
      {
        WRN("read 0 bytes");
      }
      return BUFFER_SOCKET_ERROR;
    }
    if (r > 0)
    {
      _end += r;
      return r;
    }
    else {
      return 0;
    }
  }
}
