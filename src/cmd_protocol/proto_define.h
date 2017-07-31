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

typedef uint16_t proto_t;
typedef uint16_t f2t_v2_seq_t;
#define STREAM_ID_LEN 16

#ifdef WIN32
#include <ws2tcpip.h>
#include <Windows.h>
#pragma warning(disable: 4200) 
#else
#include <sys/time.h>
#include <netinet/in.h>
#endif

enum
{
  //forward <----> forward v1
  // used in forward_server and forward_client.
  //1~24
  CMD_FC2FS_REQ_STREAM = 1,
  CMD_FS2FC_RSP_STREAM = 2,
  CMD_FS2FC_STREAMING_HEADER = 3,
  CMD_FS2FC_STREAMING = 4,
  CMD_FC2FS_KEEPALIVE = 5,
  CMD_FC2FS_UNREQ_STREAM = 6,

  //forward <----> forward v2
  // used in forward_server_v2 and forward_client_v2, but only support short stream id,
  // already replaced by f2f v3.
  CMD_FC2FS_START_TASK = 7,
  CMD_FS2FC_START_TASK_RSP = 8,
  CMD_FS2FC_STREAM_DATA = 9,
  CMD_F2F_STOP_TASK = 10,
  //CMD_F2F_STOP_TASK_RSP = 11,

  //forward <----> forward v3
  // still used in forward_server_v2 and forward_clinet_v2, support long stream id.
  //25~49
  CMD_FC2FS_START_TASK_V3 = 25,
  CMD_F2F_STOP_TASK_V3 = 26,
  CMD_FS2FC_START_TASK_RSP_V3 = 27,
  CMD_FS2FC_STREAM_DATA_V3 = 28,

  //forward <----> portal
  //50~99
  CMD_F2P_KEEPALIVE = 50,
  CMD_P2F_INF_STREAM = 51,
  CMD_P2F_START_STREAM = 52,
  CMD_P2F_CLOSE_STREAM = 53,
  CMD_P2F_INF_STREAM_V2 = 54,
  CMD_P2F_START_STREAM_V2 = 55,

  //receiver <----> portal
  //100~149
  CMD_R2P_KEEPALIVE = 100,

  //uploader<--->up_sche
  //150~200
  CMD_U2US_REQ_ADDR = 150,
  CMD_US2U_RSP_ADDR = 151,

  //tracker<--->forward
  //200~249
  CMD_F2T_REGISTER_REQ = 200,
  CMD_F2T_REGISTER_RSP = 201,
  CMD_F2T_ADDR_REQ = 202,
  CMD_F2T_ADDR_RSP = 203,
  CMD_F2T_UPDATE_STREAM_REQ = 204,
  CMD_F2T_UPDATE_STREAM_RSP = 205,
  CMD_F2T_KEEP_ALIVE_REQ = 206,
  CMD_F2T_KEEP_ALIVE_RSP = 207,

  //tracker<--->forward v2
  //250~299
  CMD_FS2T_REGISTER_REQ_V2 = 250,
  CMD_FS2T_REGISTER_RSP_V2 = 251,
  CMD_FC2T_ADDR_REQ_V2 = 252,
  CMD_FC2T_ADDR_RSP_V2 = 253,
  CMD_FS2T_UPDATE_STREAM_REQ_V2 = 254,
  CMD_FS2T_UPDATE_STREAM_RSP_V2 = 255,
  CMD_FS2T_KEEP_ALIVE_REQ_V2 = 256,
  CMD_FS2T_KEEP_ALIVE_RSP_V2 = 257,

  //tracker<--->forward v3 (support long stream id)
  //250~299
  CMD_FS2T_REGISTER_REQ_V3 = 270,
  CMD_FS2T_REGISTER_RSP_V3 = 271,
  CMD_FC2T_ADDR_REQ_V3 = 272,
  CMD_FC2T_ADDR_RSP_V3 = 273,
  CMD_FS2T_UPDATE_STREAM_REQ_V3 = 274,
  CMD_FS2T_UPDATE_STREAM_RSP_V3 = 275,
  CMD_FS2T_KEEP_ALIVE_REQ_V3 = 276,
  CMD_FS2T_KEEP_ALIVE_RSP_V3 = 277,

  //receiver<--->up_sche
  //300~350
  CMD_US2R_REQ_UP = 300,
  CMD_R2US_RSP_UP = 301,
  CMD_R2US_KEEPALIVE = 302,

  //uploader<--->receiver
  //350~399
  CMD_U2R_REQ_STATE = 350,
  CMD_U2R_RSP_STATE = 351,
  CMD_U2R_STREAMING = 352,
  CMD_U2R_CMD = 353,

  CMD_U2R_REQ_STATE_V2 = 354,
  CMD_U2R_RSP_STATE_V2 = 355,
  CMD_U2R_STREAMING_V2 = 356,
  CMD_U2R_CMD_V2 = 357,

  // mod_tracker  <--> tracker
  CMD_MT2T_REQ_RSP = 400,

  // rtp uploader <----> receiver
  CMD_RTP_U2R_REQ_STATE = 500,
  CMD_RTP_U2R_RSP_STATE = 501,
  CMD_RTP_U2R_PACKET = 502,
  CMD_RTCP_U2R_PACKET = 503,

  // rtp downloader <----> player
  CMD_RTP_D2P_REQ_STATE = 600,
  CMD_RTP_D2P_RSP_STATE = 601,
  CMD_RTP_D2P_PACKET = 602,
  CMD_RTCP_D2P_PACKET = 603,

  // rtp forward <----> forward
  CMD_RTP_F2F_REQ_STATE = 700,

};

static const int32_t TRACKER_RSP_RESULT_OK = 0;
static const int32_t TRACKER_RSP_RESULT_BADREQ = 1;
static const int32_t TRACKER_RSP_RESULT_NORESULT = 2;
static const int32_t TRACKER_RSP_RESULT_INNER_ERR = 3;

static const int32_t FORWARD_RSP_RESULT_OK = 0;
static const int32_t FORWARD_RSP_RESULT_BADREQ = 1;
static const int32_t FORWARD_RSP_RESULT_NORESULT = 2;
static const int32_t FORWARD_RSP_RESULT_INNER_ERR = 3;

static const int32_t FORWARD_SESSION_PROG_START = 0;
static const int32_t FORWARD_SESSION_PROG_IN_PROGRESS = 1;
static const int32_t FORWARD_SESSION_PROG_NOT_DATA = 2;
static const int32_t FORWARD_SESSION_PROG_BADREQ = 3;
static const int32_t FORWARD_SESSION_PROG_INNER_ERR = 4;
static const int32_t FORWARD_SESSION_PROG_WAIT = 5;//Need to wait for the data
static const int32_t FORWARD_SESSION_PROG_END = 6;

typedef enum
{
  UPD_CMD_DEL = 0,
  UPD_CMD_ADD
}update_cmd;

typedef enum
{
  PAYLOAD_TYPE_MEDIA_BEGIN = 0,
  PAYLOAD_TYPE_FLV = 0,
  PAYLOAD_TYPE_MPEGTS = 1,
  PAYLOAD_TYPE_RTP = 2,

  PAYLOAD_TYPE_MEDIA_END = 31,
  PAYLOAD_TYPE_HEARTBEAT = 32,
}payload_type;

#pragma pack(1)
typedef struct proto_header
{
  uint8_t magic;
  uint8_t version;
  uint16_t cmd;
  uint32_t size;
}proto_header;

typedef struct media_header
{
  uint32_t streamid;
  uint32_t payload_size;
  uint8_t payload_type;
  uint8_t payload[0];
}media_header;

typedef struct media_block
{
  uint32_t streamid;
  uint64_t seq;
  uint32_t payload_size;
  uint8_t payload_type;
  uint8_t payload[0];
}media_block;

typedef struct media_header_v2
{
  uint8_t streamid[STREAM_ID_LEN];
  uint32_t payload_size;
  uint8_t payload_type;
  uint8_t payload[0];
}media_header_v2;

typedef struct media_block_v2
{
  uint8_t streamid[STREAM_ID_LEN];
  uint64_t seq;
  uint32_t payload_size;
  uint8_t payload_type;
  uint8_t payload[0];
}media_block_v2;

typedef struct payload_heartbeat
{
  uint64_t src_tick;
}payload_heartbeat;

typedef struct block_map
{
  int32_t last_keyframe_ts;
  uint32_t last_keyframe_start;
  int32_t last_ts;
  media_block data;
}block_map;

typedef struct ip4_addr
{
  uint32_t ip;
  uint16_t port;
}ip4_addr;

//uploader<------->receiver
typedef struct u2r_req_state
{
  uint32_t version;
  uint32_t streamid;
  uint64_t user_id;
  uint8_t token[32];
  uint8_t payload_type;
}u2r_req_state;

enum
{
  U2R_RESULT_SUCCESS = 0,
  U2R_RESULT_INVALID_TOKEN = 1,
  U2R_RESULT_INVALID_STREAMID = 2,
  U2R_RESULT_INVALID_IP = 3,
};

typedef struct u2r_rsp_state
{
  uint32_t streamid;
  uint16_t result; //0 success; 1 token invalid; 2 streamid invalid;
}u2r_rsp_state;

typedef struct u2r_streaming
{
  uint32_t streamid;
  uint32_t payload_size;
  uint8_t payload_type;
  uint8_t payload[0];
}u2r_streaming;

typedef struct u2r_cmd
{
  uint32_t streamid;
  uint8_t action;
}u2r_cmd;

typedef struct u2r_req_state_v2
{
  uint32_t version;
  uint8_t streamid[STREAM_ID_LEN];
  uint64_t user_id;
  uint8_t token[32];
  uint8_t payload_type;
}u2r_req_state_v2;

typedef struct u2r_rsp_state_v2
{
  uint8_t streamid[STREAM_ID_LEN];
  uint16_t result; //0 success; 1 token invalid; 2 streamid invalid;
}u2r_rsp_state_v2;

typedef struct u2r_streaming_v2
{
  uint8_t streamid[STREAM_ID_LEN];
  uint32_t payload_size;
  uint8_t payload_type;
  uint8_t payload[0];
}u2r_streaming_v2;

//portal<-------->receiver
typedef struct receiver_stream_status
{
  uint32_t streamid;
  uint32_t forward_cnt;
  uint32_t last_ts;
  uint64_t block_seq;
}receiver_stream_status;

typedef struct r2p_keepalive
{
  ip4_addr listen_uploader_addr;
  uint32_t outbound_speed;
  uint32_t inbound_speed;
  uint32_t stream_cnt;
  receiver_stream_status streams[0];
}r2p_keepalive;

//forward<---->forward
typedef struct f2f_req_stream
{
  uint32_t streamid;
  uint64_t last_block_seq;
}f2f_req_stream;

typedef struct f2f_unreq_stream
{
  uint32_t streamid;
}f2f_unreq_stream;

typedef struct f2f_rsp_stream
{
  uint32_t streamid;
  uint16_t result;
}f2f_rsp_stream;

typedef struct f2f_keepalive_rsp
{
  uint16_t result;
}f2f_keepalive_rsp;

typedef struct media_header f2f_streaming_header;
typedef struct media_block f2f_streaming;

typedef struct f2f_start_task
{
  uint32_t task_id;
  uint32_t stream_id;
  uint32_t payload_size;/*sizeof(what)*/
  uint8_t payload[0]; /*what*/
}f2f_start_task;

typedef struct f2f_start_task_rsp
{
  uint32_t task_id;
  uint32_t result;     // 0: OK; 1: already existed; 2: error occur
}f2f_start_task_rsp;

typedef struct f2f_stop_task
{
  uint32_t task_id;
  uint32_t result;/*none*/
}f2f_stop_task;

typedef struct f2f_stop_task_rsp
{
  uint32_t task_id;
  uint32_t result;     // 0: OK; 1: already existed; 2: error occur
}f2f_stop_task_rsp;

typedef struct  f2f_trans_data
{
  uint32_t task_id;
  uint32_t payload_size;
  uint8_t payload[0];
}f2f_trans_data;

enum F2FStartTaskResult
{
  F2FStartTaskResultOK = 0,
  F2FStartTaskResultAlreadyExisted,
  F2FStartTaskResultError
};

//forward<---->forward v3(support long stream id)
typedef struct f2f_req_stream_v3
{
  uint8_t streamid[STREAM_ID_LEN];
  uint64_t last_block_seq;
}f2f_req_stream_v3;

typedef struct f2f_unreq_stream_v3
{
  uint8_t streamid[STREAM_ID_LEN];
}f2f_unreq_stream_v3;

typedef struct f2f_rsp_stream_v3
{
  uint8_t streamid[STREAM_ID_LEN];
  uint16_t result;
}f2f_rsp_stream_v3;

struct f2f_keepalive_rsp_v3 : public f2f_keepalive_rsp
{
};

typedef struct media_header_v2 f2f_streaming_header_v3;
typedef struct media_block_v2 f2f_streaming_v3;

typedef struct f2f_start_task_v3
{
  uint32_t task_id;
  uint8_t streamid[STREAM_ID_LEN];
  uint32_t payload_size;/*sizeof(what)*/
  uint8_t payload[0]; /*what*/
}f2f_start_task_v3;

struct f2f_start_task_rsp_v3 : public f2f_start_task_rsp
{
};

struct f2f_stop_task_v3 : public f2f_stop_task
{
};

struct f2f_stop_task_rsp_v3 : public f2f_stop_task_rsp
{
};



//portal<---->forward
typedef struct forward_stream_status
{
  uint32_t streamid;
  uint32_t player_cnt;
  uint32_t forward_cnt;
  int32_t last_ts;
  uint64_t last_block_seq;
}forward_stream_status;

typedef struct f2p_keepalive
{
  ip4_addr listen_player_addr;
  uint32_t out_speed;         //bytes per second
  uint32_t stream_cnt;
  forward_stream_status stream_status[0];
}f2p_keepalive;

typedef struct p2f_inf_stream
{
  uint16_t cnt;
  uint32_t streamids[0];
}p2f_inf_stream;

typedef struct p2f_start_stream
{
  uint32_t streamid;
}p2f_start_stream;

typedef struct p2f_close_stream
{
  uint32_t streamid;
}p2f_close_stream;

//typedef struct timeval_128
//{
//    int64_t tv_sec;
//    int64_t tv_usec;
//}timeval_128;

typedef struct stream_info
{
  uint32_t streamid;
  struct timeval start_time;

  stream_info& operator=(const stream_info& right)
  {
    this->streamid = right.streamid;
    this->start_time.tv_sec = right.start_time.tv_sec;
    this->start_time.tv_usec = right.start_time.tv_usec;

    return *this;
  }
}stream_info;


typedef struct p2f_inf_stream_v2
{
  uint16_t cnt;
  stream_info stream_infos[0];
}p2f_inf_stream_v2;

typedef stream_info p2f_start_stream_v2;

//uploader<--->up_sche
typedef struct u2us_req_addr
{
  uint32_t version;           // uploader version
  uint32_t uploader_code;     // uploader code
  uint32_t roomid;
  uint32_t streamid;
  uint64_t user_id;
  uint8_t sche_token[32];
  uint8_t up_token[32];
  uint32_t ip;
  uint16_t port;
  uint8_t stream_type;        // stream type such as flv or flv_without_audio or flv_without_video or flv_need_encode
}u2us_req_addr;

typedef struct us2u_rsp_addr
{
  uint32_t streamid;
  uint32_t ip;
  uint16_t port;
  uint16_t result;            // 0 standard for success or else for error code
  uint8_t up_token[32];
} us2u_rsp_addr;

// forward<--->tracker
typedef struct f2t_register_req
{
  uint16_t port;
  uint32_t ip;
  uint16_t asn;               /* tel or cnc */
  uint16_t region;            /* forward server room num */
}f2t_register_req_t;

typedef struct f2t_register_rsp
{
  uint16_t result;
}f2t_register_rsp_t;

typedef struct f2t_addr_req
{
  uint32_t streamid;
  uint32_t ip;
  uint16_t port;
  uint16_t asn;
  uint16_t region;
  uint16_t level;
}f2t_addr_req;

typedef struct f2t_addr_rsp
{
  uint32_t ip;
  uint16_t port;
  uint16_t result;
  uint16_t level;
}f2t_addr_rsp;

typedef struct f2t_update_stream_req
{
  uint16_t cmd;               /* 0: del; 1: add */
  uint32_t streamid;
  //uint32_t src;        /* 1: is; 0: is not */
  uint16_t level;
}f2t_update_stream_req;

typedef struct f2t_update_stream_rsp
{
  uint16_t result;
}f2t_update_stream_rsp;

typedef struct f2t_keep_alive_req
{
  uint64_t fid;               /* forward_server's id */
}f2t_keep_alive_req;

typedef struct f2t_keep_alive_rsp
{
  uint16_t result;
}f2t_keep_alive_rsp;

// forward<--->tracker v2

struct mt2t_proto_header
{
  f2t_v2_seq_t seq;
  uint16_t reserved;
};

struct f2t_register_req_v2 : public f2t_register_req
{
};

struct f2t_register_rsp_v2 : public f2t_register_rsp
{
};

struct f2t_addr_req_v2 : public f2t_addr_req
{
};

struct f2t_addr_rsp_v2 : public f2t_addr_rsp
{
};

struct f2t_update_stream_req_v2 : public f2t_update_stream_req
{
};

struct f2t_update_stream_rsp_v2 : public f2t_update_stream_rsp
{
};

struct f2t_keep_alive_req_v2 : public f2t_keep_alive_req
{
};

struct f2t_keep_alive_rsp_v2 : public f2t_keep_alive_rsp
{
};

// forward<--->tracker v3 (support long stream id)
struct f2t_register_req_v3 : public f2t_register_req
{
};

struct f2t_register_rsp_v3 : public f2t_register_rsp
{
};

typedef struct f2t_addr_req_v3
{
  uint8_t streamid[STREAM_ID_LEN];
  uint32_t ip;
  uint16_t port;
  uint16_t asn;
  uint16_t region;
  uint16_t level;
}f2t_addr_req_v3;

struct f2t_addr_rsp_v3 : public f2t_addr_rsp
{
};

typedef struct f2t_update_stream_req_v3
{
  uint16_t cmd;
  uint8_t streamid[STREAM_ID_LEN];
  uint16_t level;
}f2t_update_stream_req_v3;

struct f2t_update_stream_rsp_v3 : public f2t_update_stream_rsp
{
};

struct f2t_keep_alive_req_v3 : public f2t_keep_alive_req
{
};

struct f2t_keep_alive_rsp_v3 : public f2t_keep_alive_rsp
{
};

//receiver<--->up_sche
typedef struct us2r_req_up
{
  uint32_t seq_id;
  uint32_t streamid;
  uint64_t user_id;
  uint8_t token[32];
}us2r_req_up;

typedef struct r2us_rsp_up
{
  uint32_t seq_id;
  uint32_t streamid;
  uint8_t result;
}r2us_rsp_up;

typedef struct r2p_keepalive r2us_keepalive;

#pragma pack()

struct proto_wrapper
{
  proto_header* header;
  void* payload;
};

