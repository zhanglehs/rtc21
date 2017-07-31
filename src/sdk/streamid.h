/**
* @file		streamid.h
* @brief	This file defines stream id format for interlive \n
* @author	songshenyi
* <pre><b>copyright: Youku</b></pre>
* <pre><b>email: </b>songshenyi@youku.com</pre>
* <pre><b>company: </b>http://www.youku.com</pre>
* <pre><b>All rights reserved.</b></pre>
* @date 2014/07/22
*
*/

#pragma once

#include <stdio.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32
#define NULL 0
#define sscanf sscanf_s
#endif

static bool output_uuid = false;
static bool little_endian = false;

#pragma pack(1)
class StreamId_Ext
{
public:
  StreamId_Ext()
  {
    id_uint64[0] = 0;
    id_uint64[1] = 0;
  }

  StreamId_Ext(const StreamId_Ext& stream_id)
  {
    id_uint64[0] = stream_id.id_uint64[0];
    id_uint64[1] = stream_id.id_uint64[1];
  }

  StreamId_Ext& operator=(const unsigned& input_id)
  {
    id_uint64[0] = 0;
    id_uint64[1] = 0;

    stream_id_32 = input_id;
    return *this;
  }

  bool operator==(const StreamId_Ext& right)
  {
    if (id_uint64[0] == right.id_uint64[0]
      && id_uint64[1] == right.id_uint64[1])
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  bool operator!=(const StreamId_Ext& right)
  {
    if (id_uint64[0] == right.id_uint64[0]
      && id_uint64[1] == right.id_uint64[1])
    {
      return false;
    }
    else
    {
      return true;
    }
  }

  StreamId_Ext& operator=(const StreamId_Ext& right)
  {
    id_uint64[0] = right.id_uint64[0];
    id_uint64[1] = right.id_uint64[1];
    return *this;
  }

  int parse(const char* str, int radix = 10)
  {
    if (strlen(str) == 32)
    {
      if (little_endian)
      {
        for (int i = 0; i < 16; i++)
        {
          int ii;
          sscanf(str + 2 * i, "%2x", &ii);
          id_char[i] = ii;
        }
      }
      else
      {
        for (int i = 0; i < 16; i++)
        {
          int ii;
          sscanf(str + 2 * i, "%2x", &ii);
          id_char[15 - i] = ii;
        }
      }
    }
    else
    {
      int id_32 = (int)strtol(str, (char **)NULL, radix);
      id_uint64[0] = 0;
      id_uint64[1] = 0;
      stream_id_32 = id_32;
    }

    return 0;
  }

  int parse(const std::string& str)
  {
    return parse(str.c_str());
  }

  char unparse_four_bits(char input) const
  {
    input &= 0x0f;
    if (input < 10)
    {
      return input + '0';
    }

    return input + 'A' - 10;
  }

  char* unparse(char* str) const
  {
    if (little_endian)
    {
      for (int i = 0; i < 16; i++)
      {
        char temp1 = id_char[i];
        temp1 >>= 4;
        str[i * 2] = unparse_four_bits(temp1);

        temp1 = id_char[i];
        temp1 &= 0x0f;
        str[i * 2 + 1] = unparse_four_bits(temp1);
      }
      str[32] = 0;
    }
    else
    {
      for (int i = 0; i < 16; i++)
      {
        char temp1 = id_char[15 - i];
        temp1 >>= 4;
        str[i * 2] = unparse_four_bits(temp1);

        temp1 = id_char[15 - i];
        temp1 &= 0x0f;
        str[i * 2 + 1] = unparse_four_bits(temp1);
      }
      str[32] = 0;
    }

    return str;
  }

  uint64_t get_hash_code() const
  {
    // CityHash64((char*)(this), sizeof(*this));
    return id_uint64[0] ^ (id_uint64[1] << 32) ^ (id_uint64[1] >> 16);
  }

  StreamId_Ext& parse_default_hash_code(uint64_t hash_code)
  {
    id_uint64[1] = 0;

    id_uint64[0] = hash_code & 0xffffffff;
    id_char[8] = (hash_code >> 32) & 0xff;
    id_char[15] = (hash_code >> 40) & 0xff;

    return *this;
  }

  std::string unparse() const
  {
    char str[40];
    unparse(str);
    return std::string(str);
  }

  unsigned get_32bit_stream_id() const
  {
    return stream_id_32;
  }

public:
  static unsigned get_32bit_stream_id(StreamId_Ext& id)
  {
    return id.stream_id_32;
  }

public:
  union
  {
    struct
    {
      unsigned stream_id_32;
      unsigned char  placeholder[12];
    };
    struct
    {
      unsigned char id_char[16];
    };
    struct
    {
      unsigned long long id_uint64[2];
    };
  };

};
#pragma pack()


