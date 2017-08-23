#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef WIN32
#include <ws2tcpip.h>
#include <Windows.h>
#else
#include <arpa/inet.h>
#endif

#include "proto_rtp_rtcp.h"
#include "proto_common.h"
#include "../util/util_common.h"
#include "../util/data_buffer.h"

using namespace live_stream_sdk;
using namespace avutil;


int32_t encode_rtp_u2r_req_state(const rtp_u2r_req_state *body, DataBuffer *obuf)
{
  if (body == NULL || obuf == NULL)
  {
    return -1;
  }

  uint32_t total_sz = sizeof(proto_header) + sizeof(rtp_u2r_req_state);
  encode_header(obuf, CMD_RTP_U2R_REQ_STATE, total_sz);

  uint32_t version = htonl(body->version);
  uint64_t user_id = net_util_htonll(body->user_id);

  obuf->append_ptr(&version, sizeof(version));
  obuf->append_ptr(&(body->streamid), sizeof(body->streamid));
  obuf->append_ptr(&user_id, sizeof(user_id));
  obuf->append_ptr(&body->token, sizeof(body->token));
  obuf->append_ptr(&body->payload_type, sizeof(uint8_t));
 
  return 0;
}

int32_t encode_rtp_u2r_req_state_ext(const rtp_u2r_req_state *body, const char* device_id, live_stream_sdk::DataBuffer *obuf)
{
	return encode_rtp_u2r_req_state(body, obuf);
}

int32_t decode_rtp_u2r_rsp_state(rtp_u2r_rsp_state *rsp, DataBuffer *in_buf)
{

  size_t actual_sz = sizeof(proto_header) + sizeof(rtp_u2r_rsp_state);
  proto_header h;

  if (0 != decode_header(in_buf, &h))
  {
    return -1;
  }

  if (in_buf->data_len() < actual_sz || h.size < actual_sz)
  {
    return -2;
  }

  if (h.cmd != CMD_RTP_U2R_RSP_STATE)
  {
    return -3;
  }
  const rtp_u2r_rsp_state *body = (const rtp_u2r_rsp_state *)
    ((const char*)(in_buf->data_ptr()) + sizeof(proto_header));

  rsp->version = ntohl(body->version);
  memcpy(rsp->streamid, body->streamid, sizeof(rsp->streamid));
  rsp->result = ntohs(body->result);

  return 0;
}

int32_t encode_rtp_u2r_packet(StreamId_Ext& stream_id, const uint8_t* body, uint16_t len, DataBuffer *obuf)
{
  if (body == NULL || obuf == NULL)
  {
    return -1;
  }

  uint32_t total_sz = sizeof(proto_header) + sizeof(rtp_u2r_stream_pkt_header) + len;
  encode_header(obuf, CMD_RTP_U2R_PACKET, total_sz);
  obuf->append_ptr(&stream_id, sizeof(StreamId_Ext));
  obuf->append_ptr(body, len);

  return 0;
}

int32_t encode_rtcp_u2r_packet(StreamId_Ext& stream_id, const uint8_t* body, uint16_t len, DataBuffer *obuf)
{
  if (body == NULL || obuf == NULL)
  {
    return -1;
  }

  uint32_t total_sz = sizeof(proto_header) + sizeof(rtp_u2r_stream_pkt_header) + len;
  encode_header(obuf, CMD_RTCP_U2R_PACKET, total_sz);
  obuf->append_ptr(&stream_id, sizeof(StreamId_Ext));
  obuf->append_ptr(body, len);

  return 0;
}

int32_t decode_rtcp_u2r_packet(StreamId_Ext& streamid, DataBuffer *in_buf)
{
  assert(in_buf->data_len() >= sizeof(proto_header));
  size_t min_sz = sizeof(proto_header);
  proto_header h;

  if (0 != decode_header(in_buf, &h))
    return -1;

  if (in_buf->data_len() < min_sz)
    return -2;
  if (h.cmd != CMD_RTCP_U2R_PACKET)
  {
    return -3;
  }

  in_buf->eat(sizeof(proto_header));
  memcpy(&streamid, in_buf->data_ptr(), sizeof(StreamId_Ext));
  in_buf->eat(sizeof(rtp_u2r_stream_pkt_header));

  return 0;
}


int32_t encode_rtp_d2p_req_state(const rtp_d2p_req_state* body, DataBuffer *obuf)
{
  if (body == NULL || obuf == NULL)
  {
    return -1;
  }

  uint32_t total_sz = sizeof(proto_header) + sizeof(rtp_d2p_req_state);
  encode_header(obuf, CMD_RTP_D2P_REQ_STATE, total_sz);

  uint32_t version = htonl(body->version);

  obuf->append_ptr(&version, sizeof(version));
  obuf->append_ptr(&(body->streamid), sizeof(body->streamid));
  obuf->append_ptr(&body->token, sizeof(body->token));
  obuf->append_ptr(&body->payload_type, sizeof(uint8_t));
  obuf->append_ptr(&body->useragent, sizeof(body->useragent));
  
  return 0;
}

int32_t encode_rtp_d2p_req_state_ext(const rtp_d2p_req_state* body, const char* device_id, live_stream_sdk::DataBuffer *obuf)
{
	return encode_rtp_d2p_req_state(body, obuf);
}

int32_t decode_rtp_d2p_rsp_state(rtp_d2p_rsp_state* rsp, DataBuffer *in_buf)
{
  size_t actual_sz = sizeof(proto_header) + sizeof(rtp_u2r_stream_pkt_header) + sizeof(rtp_d2p_rsp_state);
  proto_header h;

  if (0 != decode_header(in_buf, &h))
  {
    return -1;
  }

  if (in_buf->data_len() < actual_sz || h.size < actual_sz)
  {
    return -2;
  }

  if (h.cmd != CMD_RTP_D2P_RSP_STATE)
  {
    return -3;
  }
  const rtp_d2p_rsp_state *body = (const rtp_d2p_rsp_state *)
    ((const char*)(in_buf->data_ptr()) + sizeof(proto_header));

  rsp->version = ntohl(body->version);
  memcpy(rsp->streamid, body->streamid, sizeof(rsp->streamid));
  rsp->result = ntohs(body->result);

  return 0;
}

int32_t encode_rtcp_d2p_packet(StreamId_Ext& stream_id, const uint8_t* body, uint16_t len, DataBuffer *obuf)
{
  if (body == NULL || obuf == NULL)
  {
    return -1;
  }

  uint32_t total_sz = sizeof(proto_header) + sizeof(rtp_u2r_stream_pkt_header) + len;
  encode_header(obuf, CMD_RTCP_D2P_PACKET, total_sz);
  obuf->append_ptr(&stream_id, sizeof(StreamId_Ext));
  obuf->append_ptr(body, len);

  return 0;
}

int32_t decode_rtcp_d2p_packet(StreamId_Ext& stream_id, DataBuffer *in_buf)
{
  assert(in_buf->data_len() >= sizeof(proto_header));
  size_t min_sz = sizeof(proto_header);
  proto_header h;

  if (0 != decode_header(in_buf, &h))
    return -1;

  if (in_buf->data_len() < min_sz)
    return -2;
  if (h.cmd != CMD_RTCP_D2P_PACKET)
  {
    return -3;
  }

  in_buf->eat(sizeof(proto_header));

  if (in_buf->data_len() < sizeof(StreamId_Ext))
  {
    return -4;
  }
  memcpy(&stream_id, in_buf->data_ptr(), sizeof(StreamId_Ext));
  in_buf->eat(sizeof(rtp_u2r_stream_pkt_header));

  return 0;
}

int32_t decode_rtp_d2p_packet(StreamId_Ext& stream_id, DataBuffer *in_buf)
{
  assert(in_buf->data_len() >= sizeof(proto_header));
  size_t min_sz = sizeof(proto_header);
  proto_header h;

  if (0 != decode_header(in_buf, &h))
    return -1;

  if (in_buf->data_len() < min_sz)
    return -2;
  if (h.cmd != CMD_RTP_D2P_PACKET)
  {
    return -3;
  }

  in_buf->eat(sizeof(proto_header));

  if (in_buf->data_len() < sizeof(StreamId_Ext))
  {
    return -4;
  }
  memcpy(&stream_id, in_buf->data_ptr(), sizeof(StreamId_Ext));
  in_buf->eat(sizeof(rtp_u2r_stream_pkt_header));

  return 0;
}
