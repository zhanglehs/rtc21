#pragma once

#include "proto_define_rtp.h"
#include "../util/data_buffer.h"
#include "../sdk/streamid.h"
#include <stdint.h>


// rtp uploader <----> rtp receiver
int32_t encode_rtp_u2r_req_state_ext(const rtp_u2r_req_state *body, const char* device_id, live_stream_sdk::DataBuffer *obuf);

//int32_t encode_rtp_u2r_req_state(const rtp_u2r_req_state *body, live_stream_sdk::RtpU2rExtension *extension, live_stream_sdk::DataBuffer *obuf);

int32_t decode_rtp_u2r_rsp_state(rtp_u2r_rsp_state *rsp, live_stream_sdk::DataBuffer *in_buf);

int32_t encode_rtp_u2r_packet(StreamId_Ext& stream_id, const uint8_t* body, uint16_t len, live_stream_sdk::DataBuffer *obuf);

int32_t encode_rtcp_u2r_packet(StreamId_Ext& stream_id, const uint8_t* body, uint16_t len, live_stream_sdk::DataBuffer *obuf);

int32_t decode_rtcp_u2r_packet(StreamId_Ext& streamid, live_stream_sdk::DataBuffer *in_buf);


// rtp downloader <----> rtp player
int32_t encode_rtp_d2p_req_state_ext(const rtp_d2p_req_state* body, const char* device_id, live_stream_sdk::DataBuffer *obuf);

//int32_t encode_rtp_d2p_req_state(const rtp_d2p_req_state* body, live_stream_sdk::RtpD2pExtension *extension, live_stream_sdk::DataBuffer *obuf);

int32_t decode_rtp_d2p_rsp_state(rtp_d2p_rsp_state* rsp, live_stream_sdk::DataBuffer *in_buf);

int32_t encode_rtcp_d2p_packet(StreamId_Ext& stream_id, const uint8_t* body, uint16_t len, live_stream_sdk::DataBuffer *obuf);

int32_t decode_rtcp_d2p_packet(StreamId_Ext& stream_id, live_stream_sdk::DataBuffer *in_buf);

int32_t decode_rtp_d2p_packet(StreamId_Ext& stream_id, live_stream_sdk::DataBuffer *in_buf);
