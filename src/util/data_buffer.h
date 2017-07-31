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
#include <string.h>
#include <pthread.h>

namespace live_stream_sdk
{
  class DataBuffer
  {
  public:
    enum BUFFER_ERROR_CODE
    {
      BUFFER_OVER_SIZE = -4,
      BUFFER_CAPACITY_ERROR = -3,
      BUFFER_MALLOC_ERROR = -2,
      BUFFER_SOCKET_ERROR = -1,
    };

    DataBuffer(uint32_t cap);
    ~DataBuffer();

    // @return length of data read 
    int append(const DataBuffer *src);
    int append(const DataBuffer *src, size_t src_len);
    int append_ptr(const void *src, size_t src_len);
    int append_byte(unsigned char byte);

    // @return length of data writen
    int copy(const DataBuffer * src);

    int write_tcp_fd(int fd);
    int write_tcp_fd_max(int fd, size_t max);
    int read_tcp_fd(int fd);
    int read_tcp_fd_max(int fd, size_t max);

    int write_buffer(DataBuffer *dst);
    int write_buffer(void *dst, size_t dst_len);

    void try_adjust();
    void adjust();

    size_t capacity() const { return _size - _end; }
    size_t free_size() const { return _size - data_len(); }
    size_t data_len() const { return _end - _start; }
    const void * data_ptr() const { return _ptr + _start; }
    void * free_ptr() { return _ptr + _end; }
    void clear() { _start = _end = 0; }
    int eat(size_t bytes);
    size_t ignore(size_t bytes);
    void cleanall()
    {
      memset(_ptr, 0, _max);
    }

  private:
    char *_ptr;
    size_t _size;
    volatile size_t _end, _start;
    size_t _max;

    pthread_mutex_t m_mutex_write_buffer;
    pthread_mutex_t m_mutex_read_buffer;
  };
}
