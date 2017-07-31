#include "proto_common.h"


using namespace live_stream_sdk;

static const uint8_t VER = 1;
static const uint8_t MAGIC = 0xff;

int encode_header(live_stream_sdk::DataBuffer * obuf, uint16_t cmd, uint32_t size)
{
  if (obuf->capacity() < size)
  {
    return -1;
  }
  uint16_t ncmd = htons(cmd);
  uint32_t nsize = htonl(size);

  obuf->append_ptr(&MAGIC, sizeof(uint8_t));
  obuf->append_ptr(&VER, sizeof(uint8_t));
  obuf->append_ptr(&ncmd, sizeof(uint16_t));
  obuf->append_ptr(&nsize, sizeof(uint32_t));

  return 0;
}

int decode_header(const live_stream_sdk::DataBuffer * ibuf, proto_header * h)
{
  if (ibuf->data_len() < sizeof(proto_header))
    return -1;
  const proto_header *ih = (const proto_header *)ibuf->data_ptr();

  if (VER != ih->version)
    return -2;

  h->magic = ih->magic;
  h->version = ih->version;
  h->cmd = ntohs(ih->cmd);
  h->size = ntohl(ih->size);
  return 0;
}
