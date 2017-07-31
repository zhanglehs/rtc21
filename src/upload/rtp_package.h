#ifndef UPLOAD_RTP_PACKAGE_H
#define UPLOAD_RTP_PACKAGE_H

#include <cstddef>
#include <vector>

class rtcRTPPackage {
public:
  struct RtpPacket {
    void *buf;
    unsigned int len;
    unsigned int ts_ms;
    bool keyframe;
    bool video;
    RtpPacket() : buf(NULL), len(0), ts_ms(0), keyframe(false), video(false) {}
  };

public:
  rtcRTPPackage(unsigned int audio_ssrc, unsigned int video_ssrc,
    unsigned int audio_frequence, unsigned int audio_frame_size);
  ~rtcRTPPackage();

  int PackAAC(RtpPacket *packet, char *data, int len, unsigned int timestamp_ms);
  int PackH264(std::vector<RtpPacket> &packets, char *data, int len,
    int width, int height, unsigned int timestamp_ms);

private:
  struct NALU_t {
    //int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    unsigned int len;             //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
    //int forbidden_bit;            //! should be always FALSE
    //int nal_reference_idc;        //! NALU_PRIORITY_xxxx
    //int nal_unit_type;            //! NALU_TYPE_xxxx
    char *buf;                    //! contains the first byte followed by the EBSP
    //unsigned int is_present;  //! true, if packet loss is detected
    NALU_t() : buf(NULL), len(0) {}
  };

  int ProcessSPS();
  void PackNalus(std::vector<RtpPacket> &packets, int width, int height, unsigned int rtp_timestamp);
  static int GetAnnexbNALU(std::vector<NALU_t> &nalu, char *buf, int len);

  unsigned short m_audio_seqnum;
  unsigned int m_audio_count;
  unsigned int m_audio_start_capture_ms;
  unsigned int m_audio_ssrc;
  bool m_audio_inited;
  std::vector<char> m_audio_rtp_buf;
  unsigned int m_audio_frequence;
  unsigned int m_audio_frame_size;

  unsigned short m_video_seqnum;
  unsigned int m_video_start_timestamp_ms;
  unsigned int m_video_ssrc;
  bool m_video_inited;
  std::vector<char> m_sps_data;
  std::vector<char> m_pps_data;
  NALU_t m_sps_info;
  NALU_t m_pps_info;
  std::vector<NALU_t> m_video_nalu;
  std::vector<char*> m_video_rtp_buf;
};

#endif
