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
#include "../avformat/rtp.h"

namespace media_manager
{
  class RTPBlock
  {
  public:
    RTPBlock();
    RTPBlock(const avformat::RTP_FIXED_HEADER* rtp, uint16_t len);

    avformat::RTP_FIXED_HEADER* init();
    avformat::RTP_FIXED_HEADER* set(const avformat::RTP_FIXED_HEADER*, uint16_t);
    avformat::RTP_FIXED_HEADER* get(uint16_t&);

    uint32_t get_ssrc() const;
    uint16_t get_seq() const;
    uint32_t get_timestamp() const;

    int get_payload_type();

    bool is_valid();
    void set_invalid();
    uint16_t len();

    void finalize();
    ~RTPBlock();

    // these functions are working for debug.
    static int get_new_count();
    static int get_delete_count();

    static int new_count;
    static int delete_count;

    uint16_t _max_len;

  protected:
    avformat::RTP_FIXED_HEADER* _rtp_header;
    uint16_t _rtp_len;
  };

}
