#pragma once

#include <stdint.h>
#include "proto_define.h"
#include "../util/data_buffer.h"

int encode_header(live_stream_sdk::DataBuffer * obuf, uint16_t cmd, uint32_t size);

int decode_header(const live_stream_sdk::DataBuffer * ibuf, proto_header * h);
