#include "../upload/rtp_package.h"

#include "../log/log.h"
#include <string.h>
#include <assert.h>
#ifdef WIN32
#define snprintf _snprintf
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#include <stdlib.h>
#endif

using namespace live_stream_sdk;

#define H264_PLT               96
#define H264_MAX_ENCODED_SIZE  500*1024
#define H264_MAX_SPSPPS_SIZE   1024
#define AAC_MAX_ENCODED_SIZE   1024

#define NAL_TYPE_SPS 7
#define NAL_TYPE_PPS 8
#define NAL_TYPE_IDR 5

#define RTP_MTU_SIZE 1400

namespace {

  typedef struct {
    /**//* byte 0 */
    unsigned char csrc_len : 4;        /**//* expect 0 */
    unsigned char extension : 1;        /**//* expect 1, see RTP_OP below */
    unsigned char padding : 1;        /**//* expect 0 */
    unsigned char version : 2;        /**//* expect 2 */

    /**//* byte 1 */
    unsigned char payload : 7;        /**//* RTP_PAYLOAD_RTSP */
    unsigned char marker : 1;        /**//* expect 1 */

    /**//* bytes 2, 3 */
    unsigned short seq_no;
    /**//* bytes 4-7 */
    unsigned  int timestamp;
    /**//* bytes 8-11 */
    unsigned int ssrc;            /**//* stream number is used here. */
  } RTP_FIXED_HEADER;

  // 扩展头的前两个字段是官方的规定，后面的字段是字定义的
  typedef struct {
    unsigned short rtp_extend_profile; //profile used
    unsigned short rtp_extend_length;  //rtp extend length
    unsigned short rtp_extend_rtplen;  //whole rtp packet length
    unsigned char padding1 : 6;
    unsigned char first_packet : 1;
    unsigned char keyframe : 1;
    unsigned char padding2;
  } EXTEND_HEADER;

  typedef struct {
    //byte 0
    unsigned char TYPE : 5;
    unsigned char NRI : 2;
    unsigned char F : 1;
  } NALU_HEADER; /**//* 1 BYTES */

  typedef struct {
    //byte 0
    unsigned char TYPE : 5;
    unsigned char NRI : 2;
    unsigned char F : 1;
  } FU_INDICATOR; /**//* 1 BYTES */

  typedef struct {
    //byte 0
    unsigned char TYPE : 5;
    unsigned char R : 1;
    unsigned char E : 1;
    unsigned char S : 1;
  } FU_HEADER; /**//* 1 BYTES */

  int AddAACRtpHeader(unsigned int ssrc, char *inBuf, int inLen, char *outBuf,
    unsigned int ts, unsigned short seq_no) {
    unsigned char head_data[sizeof(RTP_FIXED_HEADER)];
    memset(head_data, 0, sizeof(head_data));
    RTP_FIXED_HEADER *fix_head = (RTP_FIXED_HEADER*)&head_data[0];
    fix_head->extension = 0;
    fix_head->version = 2;
    fix_head->payload = 97;
    fix_head->marker = 0;
    fix_head->seq_no = htons(seq_no);
    fix_head->timestamp = htonl(ts);
    fix_head->ssrc = htonl(ssrc);

    memcpy(outBuf, head_data, sizeof(head_data));
    memcpy(outBuf + sizeof(head_data), inBuf, inLen);
    return inLen + sizeof(head_data);
  }

  int FindNaluStartCode(char *buf) {
    int start_len = 0;
    if (buf[0] == 0 && buf[1] == 0) {
      if (buf[2] == 1) {
        start_len = 3;
      }
      else if (buf[2] == 0 && buf[3] == 1) {
        start_len = 4;
      }
    }
    return start_len;
  }

  void FillH264RtpFixedHeader(char *buf, unsigned int ssrc,
    unsigned short seqnum, unsigned long ts) {
    RTP_FIXED_HEADER *header = (RTP_FIXED_HEADER*)buf;
    header->csrc_len = 0;       //                       CC
    header->extension = 1;      //                       X
    header->padding = 0;        //                       P
    header->version = 2;        //版本号，此版本固定为2  V
    header->payload = H264_PLT; //负载类型号
    header->marker = 0;         //标志位，由具体协议规定其值。M 不同的有效载荷有不同的含义，对于视频，标记一帧的结束；对于音频，标记会话的开始。
    header->seq_no = htons(seqnum);
    header->timestamp = htonl(ts);
    header->ssrc = htonl(ssrc);
  }

  void FillH264RtpExtendHeader(char *buf, int width, int height,
    bool first_packet, int nal_unit_type) {
    EXTEND_HEADER * header = (EXTEND_HEADER *)buf;
    memset(header, 0, sizeof(EXTEND_HEADER));
    header->rtp_extend_profile = 0;
    int rtp_extend_length = (sizeof(EXTEND_HEADER) >> 2) - 1; //rtp_extend_profile,rtp_extend_length之外1个字（4个字节）
    header->rtp_extend_length = htons(rtp_extend_length);
    bool is_key_frame = ((nal_unit_type == 7) || (nal_unit_type == 8)
      || (nal_unit_type == 6) || (nal_unit_type == 5));
    header->first_packet = first_packet;
    header->keyframe = is_key_frame;
  }

  void GetNaluInfo(char *nalu_buf, int *nal_unit_type,
    int *forbidden_bit = NULL, int *nal_reference_idc = NULL) {
    if (forbidden_bit) {
      *forbidden_bit = *nalu_buf & 0x80; //1 bit
    }
    if (nal_reference_idc) {
      *nal_reference_idc = *nalu_buf & 0x60; // 2 bit
    }
    if (nal_unit_type) {
      *nal_unit_type = *nalu_buf & 0x1f;// 5 bit
    }
  }

}

rtcRTPPackage::rtcRTPPackage(unsigned int audio_ssrc, unsigned int video_ssrc,
  unsigned int audio_frequence, unsigned int audio_frame_size) {
  m_audio_ssrc = audio_ssrc;
  m_audio_seqnum = 0;
  m_audio_count = 0;
  m_audio_start_capture_ms = 0;
  m_audio_inited = false;
  m_audio_frequence = audio_frequence;
  m_audio_frame_size = audio_frame_size;

  m_video_ssrc = video_ssrc;
  m_video_seqnum = 0;
  m_video_start_timestamp_ms = 0;
  m_video_inited = false;
  m_sps_data.resize(32);
  m_pps_data.resize(32);
}

rtcRTPPackage::~rtcRTPPackage() {
  for (auto it = m_video_rtp_buf.begin(); it != m_video_rtp_buf.end(); it++) {
    delete[] *it;
  }
}

int rtcRTPPackage::PackAAC(RtpPacket *packet, char *encode_buf,
  int encode_len, unsigned int capture_ms) {
  packet->buf = NULL;
  packet->len = 0;

  if (encode_len > AAC_MAX_ENCODED_SIZE) {
    char msg[128];
    sprintf(msg, "too large aac frame size %d", encode_len);
    ERR(msg);
    //LiveStreamSDKOpenSendReport("AAC_RTP_ERROR", msg, __FUNCTION__);
    return -1;
  }

  if (!m_audio_inited) {
    m_audio_inited = true;
    m_audio_start_capture_ms = capture_ms;
  }

  // 一、rtp时间戳的有哪些需求：
  // 两种计算时间戳的方法，
  // 1. 根据采集ms计算时间戳（以48k采样为例，每秒的rtp时间戳应增长48000）
  // 2. 根据采样点数计算的时间戳（rtp时间戳代表的其实是采集点数，每个rtp包的采样点数为m_audio_frame_size，于是rtp时间戳可以由递增m_audio_frame_size得到）
  // 上述两个时间戳的计算方法，从长时间尺度上来看，会有偏差，或者ios从后台恢复时也会有偏差。
  // webrtc的播放器接受第2种时间戳，而服务器转码接受第1种时间戳（服务器不希望解析rtcp，仅仅是将rtp_timestamp/(m_audio_frequence/1000)作为flv的时间戳），
  // 为了同时支持webrtc播放和服务器转码，需要对两者的偏差进行纠正。
  // 二、rtp时间戳的纠正思路
  // 为保证服务器转码的正常运行，在大尺度上应按方法1计算时间戳，而为保证webrtc播放，在微观上应按方法2计算时间戳。
  // 具体操作是，按方法1和方法2分别计算时间戳，
  // 当两者差异 < 50ms时，以方法2为准；
  // 当两者差异在[50 - 100]ms时，将方法2的计算值微调，微调m_audio_frame_size的量，使其更靠近方法1的值；
  // 当两者差异 > 100ms时，以方法1的计算为准，但保证时间戳为m_audio_frame_size的整数倍。
  // 三、限制条件
  // 通常来说，音频采集设备是很稳定的，即capture_ms会按均匀间隔递增，此时不会有什么问题。
  // 但如果某些android设备输出的capture_ms以非常大的波动递增（例如递增间隔为1ms、1ms、120ms、+10ms、2ms、...）
  // 此时会反复触发rtp时间戳的调整，最终webrtc播放时收到的rtp时间戳基本是乱套的，最终输出的音频质量其差。
  // 这与capture_ms的计算很有关系，某些平台的音频采集回调api中会带上采集时间参数，此时应优先选择该参数为capture_ms；
  // 否则应选取采集回调时的系统时间为capture_ms，而不能选取编码完成后的系统时间为capture_ms。
  // 采用上述capture_ms的计算方法，到目前为止还没有碰到capture_ms波动非常大的情况。

  int rtp_timestamp_1ms = m_audio_frequence / 1000; // 每1ms对应rtp时间戳的增长量
  int rtp_timestamp_100ms = rtp_timestamp_1ms * 100;

  // 以采集时间计算的rtp时间戳
  unsigned int rtp_ts_capture = (capture_ms - m_audio_start_capture_ms)
    * rtp_timestamp_1ms;
  // 以采样点累加的rtp时间戳
  unsigned int rtp_ts_sample = m_audio_count * m_audio_frame_size;
  // 两个rtp时间戳的差值为理论调整量
  int diff = int(rtp_ts_capture - rtp_ts_sample);

  if (abs(diff) > rtp_timestamp_100ms) {
    // 偏差量太大了（大于100ms），以采样时间为准，但将rtp时间戳调整为m_audio_frame_size整数倍
    m_audio_count = rtp_ts_capture / m_audio_frame_size;
    rtp_ts_sample = m_audio_count * m_audio_frame_size;
  }
  else {
    // 偏差量较小（50-100ms），每次微调1个rtp包的时间戳（即m_audio_frame_size大小）
    int rtp_timestamp_50ms = rtp_timestamp_1ms * 50;
    if (diff >= rtp_timestamp_50ms) {
      m_audio_count++;
      rtp_ts_sample += m_audio_frame_size;
    }
    else if (diff <= -rtp_timestamp_50ms) {
      m_audio_count--;
      rtp_ts_sample -= m_audio_frame_size;
    }
  }
  m_audio_count++;

  if ((int)m_audio_rtp_buf.size() < encode_len + 100) {
    m_audio_rtp_buf.resize(encode_len + 100);
  }
  packet->buf = &m_audio_rtp_buf[0];
  packet->len = AddAACRtpHeader(m_audio_ssrc, encode_buf, encode_len,
    &m_audio_rtp_buf[0], rtp_ts_sample, m_audio_seqnum++);
  packet->ts_ms = rtp_ts_sample / (m_audio_frequence / 1000);
  packet->keyframe = true;
  return 0;
}

int rtcRTPPackage::PackH264(std::vector<RtpPacket> &packets, char *encode_buf,
  int encode_len, int width, int height, unsigned int timestamp_ms) {
  packets.clear();

  //static unsigned int s_old_ts_ms = (unsigned int)-1;
  //assert(s_old_ts_ms != timestamp_ms);
  //s_old_ts_ms = timestamp_ms;

  if (encode_buf == NULL || encode_len <= 0 || encode_len > H264_MAX_ENCODED_SIZE) {
    char msg[128];
    sprintf(msg, "param error, data=%p, len=%d", encode_buf, encode_len);
    ERR(msg);
    //LiveStreamSDKOpenSendReport("H264_RTP_ERROR", msg, __FUNCTION__);
    return -1;
  }

  if (GetAnnexbNALU(m_video_nalu, encode_buf, encode_len) != 0) {
    return -1;
  }

  if (ProcessSPS() != 0) {
    return -1;
  }

  if (m_video_nalu.empty()) {
    return 0;
  }

  if (!m_video_inited) {
    m_video_inited = true;
    m_video_start_timestamp_ms = timestamp_ms;
  }
  unsigned int rtp_timestamp = (timestamp_ms - m_video_start_timestamp_ms) * 90;
  PackNalus(packets, width, height, rtp_timestamp);

  return 0;
}

int rtcRTPPackage::ProcessSPS() {
  for (auto it = m_video_nalu.begin(); it != m_video_nalu.end();) {
    if (it->len <= 0) {
      it = m_video_nalu.erase(it);
      continue;
    }

    int nal_unit_type = 0;
    GetNaluInfo(it->buf, &nal_unit_type);
    if (nal_unit_type == NAL_TYPE_SPS || nal_unit_type == NAL_TYPE_PPS) {
      if (it->len >= H264_MAX_SPSPPS_SIZE) {
        char msg[128];
        sprintf(msg, "sps/pps size error, nalu_type=%d, size=%d", nal_unit_type, it->len);
        ERR(msg);
        //LiveStreamSDKOpenSendReport("H264_RTP_ERROR", msg, __FUNCTION__);
        return -1;
      }

      if (nal_unit_type == NAL_TYPE_SPS) {
        m_sps_info = *it;
        if (m_sps_info.len > m_sps_data.size()) {
          m_sps_data.resize(m_sps_info.len);
        }
        memcpy(&m_sps_data[0], m_sps_info.buf, m_sps_info.len);
        m_sps_info.buf = &m_sps_data[0];
      }
      else if (nal_unit_type == NAL_TYPE_PPS) {
        m_pps_info = *it;
        if (m_pps_info.len > m_pps_data.size()) {
          m_pps_data.resize(m_pps_info.len);
        }
        memcpy(&m_pps_data[0], m_pps_info.buf, m_pps_info.len);
        m_pps_info.buf = &m_pps_data[0];
      }
      it = m_video_nalu.erase(it);
      continue;
    }

    it++;
  }
  if (m_video_nalu.empty()) {
    return 0;
  }

  int nal_unit_type = 0;
  GetNaluInfo(m_video_nalu[0].buf, &nal_unit_type);
  if (nal_unit_type == NAL_TYPE_IDR) {
    if (m_sps_info.len <= 0 || m_pps_info.len <= 0) {
      m_video_nalu.clear();
      return 0;
    }
    m_video_nalu.insert(m_video_nalu.begin(), m_pps_info);
    m_video_nalu.insert(m_video_nalu.begin(), m_sps_info);
  }
  return 0;
}

void rtcRTPPackage::PackNalus(std::vector<RtpPacket> &packets, int width,
  int height, unsigned int rtp_timestamp) {
  bool first_nalu = true;
  bool last_nalu = false;
  char *outBuf = NULL;
  unsigned int packet_count = 0;

  for (auto it = m_video_nalu.begin(); it != m_video_nalu.end(); it++) {
    last_nalu = (it == (--m_video_nalu.end()));
    int nal_unit_type = 0;
    int forbidden_bit = 0;
    int nal_reference_idc = 0;
    GetNaluInfo(it->buf, &nal_unit_type, &forbidden_bit, &nal_reference_idc);
    int packet_count_of_nal = (it->len + RTP_MTU_SIZE - 1) / RTP_MTU_SIZE;
    bool first_packet_of_nal = true;
    bool last_packet_of_nal = false;

    while (it->len > 0) {
      int packet_len = 0;
      int payload_len = RTP_MTU_SIZE;
      if (it->len <= RTP_MTU_SIZE) {
        last_packet_of_nal = true;
        payload_len = it->len;
      }

      packet_count++;
      if (packet_count > m_video_rtp_buf.size()) {
        m_video_rtp_buf.resize(packet_count);
        m_video_rtp_buf[packet_count - 1] = new char[RTP_MTU_SIZE + 100];
      }
      outBuf = m_video_rtp_buf[packet_count - 1];

      int header_len = 0;
      RTP_FIXED_HEADER *rtp_hdr = (RTP_FIXED_HEADER*)outBuf;
      FillH264RtpFixedHeader(outBuf, m_video_ssrc, m_video_seqnum++, rtp_timestamp);
      header_len += sizeof(RTP_FIXED_HEADER);
      if (last_nalu && last_packet_of_nal) {
        rtp_hdr->marker = 1;
      }
      EXTEND_HEADER *ext_head = NULL;
      if (rtp_hdr->extension) {
        ext_head = (EXTEND_HEADER*)(outBuf + header_len);
        FillH264RtpExtendHeader(outBuf + header_len, width, height,
          first_nalu && first_packet_of_nal, nal_unit_type);//RTP extension Header
        header_len += sizeof(EXTEND_HEADER);
      }

      if (packet_count_of_nal == 1) {
        char *payload = outBuf + header_len;
        NALU_HEADER* nalu_hdr = (NALU_HEADER*)payload;
        nalu_hdr->F = forbidden_bit;
        nalu_hdr->NRI = nal_reference_idc >> 5;//有效数据在n->nal_reference_idc的第6，7位，需要右移5位才能将其值赋给nalu_hdr->NRI。
        nalu_hdr->TYPE = nal_unit_type;
        memcpy(payload + 1, it->buf + 1, payload_len - 1); //not include forbidden_zero_bit nal_ref_idc and nal_unit_type(1 byte)
        packet_len = header_len + payload_len;
      }
      else {
        char *payload = outBuf + header_len;
        FU_INDICATOR *fu_ind = (FU_INDICATOR*)payload;
        fu_ind->F = 0;
        fu_ind->NRI = nal_reference_idc >> 5;
        fu_ind->TYPE = 28;

        FU_HEADER *fu_hdr = (FU_HEADER*)(payload+1);
        fu_hdr->S = (int)first_packet_of_nal;
        fu_hdr->R = 0;
        fu_hdr->E = (int)last_packet_of_nal;
        fu_hdr->TYPE = nal_unit_type;

        if (first_packet_of_nal) {
          memcpy(payload + 2, it->buf + 1, payload_len - 1);
          packet_len = header_len + payload_len + 1;
        }
        else {
          memcpy(payload + 2, it->buf, payload_len);
          packet_len = header_len + payload_len + 2;
        }
      }
      if (rtp_hdr->extension) {
        ext_head->rtp_extend_rtplen = htons(packet_len);
      }

      packets.resize(packet_count);
      packets[packet_count - 1].buf = outBuf;
      packets[packet_count - 1].len = packet_len;
      packets[packet_count - 1].ts_ms = rtp_timestamp / 90;
      packets[packet_count - 1].keyframe
        = ((nal_unit_type == NAL_TYPE_IDR)
        || (nal_unit_type == NAL_TYPE_SPS)
        || (nal_unit_type == NAL_TYPE_PPS));
      packets[packet_count - 1].video = true;

      it->buf += payload_len;
      it->len -= payload_len;
      first_packet_of_nal = false;
    }

    first_nalu = false;
  }
}

int rtcRTPPackage::GetAnnexbNALU(std::vector<NALU_t> &nalu, char *buf, int len) {
  nalu.clear();

  if (len < 4) {
    char msg[128];
    sprintf(msg, "frame size too small, size=%d", len);
    ERR(msg);
    //LiveStreamSDKOpenSendReport("H264_RTP_ERROR", msg, __FUNCTION__);
    return -1;
  }

  int startcodeprefix_len = FindNaluStartCode(buf);
  if (startcodeprefix_len == 0) {
    char msg[128];
    sprintf(msg, "frame data not start with 0x000001 or 0x00000001");
    ERR(msg);
    //LiveStreamSDKOpenSendReport("H264_RTP_ERROR", msg, __FUNCTION__);
    return -1;
  }

  buf += startcodeprefix_len;
  len -= startcodeprefix_len;
  char *nal_start = buf;

  while (len >= 4) {
    startcodeprefix_len = FindNaluStartCode(buf);
    if (startcodeprefix_len == 0) {
      buf++;
      len--;
      continue;
    }

    nalu.resize(nalu.size() + 1);
    NALU_t &nal = nalu[nalu.size() - 1];
    nal.buf = nal_start;
    nal.len = static_cast<int>(buf - nal_start);

    nal_start = buf + startcodeprefix_len;
    buf += startcodeprefix_len;
    len -= startcodeprefix_len;
  }

  if (len == 3 && buf[0] == 0 && buf[1] == 0 && buf[2] == 1) {
  }
  else {
    buf += len;
  }
  nalu.resize(nalu.size() + 1);
  NALU_t &nal = nalu[nalu.size() - 1];
  nal.buf = nal_start;
  nal.len = static_cast<int>(buf - nal_start);
  return 0;
}
