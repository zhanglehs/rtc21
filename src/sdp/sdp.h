#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <sstream>

namespace live_stream_sdk
{
  enum RTPTransport
  {
    RTP_AVP = 1,
    RTP_AVPF = 2,
  };

  enum RTPAVType
  {
    RTP_AV_NULL = 0,
    RTP_AV_MP3 = 14,
    RTP_AV_H264 = 96,
    RTP_AV_AAC = 97,
    RTP_AV_FLEXIBLE_FEC = 126,
    RTP_AV_FEC = 127,
    RTP_AV_ALL = 255,
  };

  class sdp_session_level
  {
  public:
    sdp_session_level& operator=(const sdp_session_level& right);

    int sdp_version;      /**< protocol version (currently 0) */
    int id;               /**< session ID */
    int version;          /**< session version */
    int start_time;       /**< session start time (NTP time, in seconds),
                          or 0 in case of permanent session */
    int end_time;         /**< session end time (NTP time, in seconds),
                          or 0 if the session is not bounded */
    int ttl;              /**< TTL, in case of multicast stream */
    std::string user;     /**< username of the session's creator */
    std::string src_addr; /**< IP address of the machine from which the session was created */
    std::string src_type; /**< address type of src_addr */
    std::string dst_addr; /**< destination IP address (can be multicast) */
    std::string dst_type; /**< destination IP address type */
    std::string name;     /**< session name (can be an empty string) */
  };

  class rtp_media_info
  {
  public:
    rtp_media_info();
    ~rtp_media_info();

    rtp_media_info(const rtp_media_info& info);

    uint32_t channels;
    uint32_t rate;  //sample rate: for video:90000  for audio:48000 or 32000 etl.

    /* save config data here
    * for AAC, this should be latm Audio Specific Config
    *       2 bytes for LC-AAC
    *       7 bytes for HE-AAC
    *
    * for H264, this should be sps/pps
    *       annexb format, have start code 0x00 00 00 01
    */
    uint8_t *extra_data;
    uint32_t extra_data_len;

    std::string dest_addr;
    uint16_t dest_port;

    uint16_t constant_duration; // for audio 1024 or 2048
    uint16_t payload_type;      // eg: RTP_AV_H264 RTP_AV_AAC
    uint32_t packetization_mode;
    uint32_t h264_profile_level_id;
    uint32_t rtp_transport;     // eg: RTP_AVP
  };

  struct SDPParseState;

  class SdpInfo
  {
  public:
    SdpInfo();
    ~SdpInfo();

    void set_sdp(const std::vector<rtp_media_info*>& media_infos, const sdp_session_level *header = NULL);
    const std::vector<rtp_media_info*>& get_media_infos();
    sdp_session_level* get_sdp_header();
    std::string load(std::string&);
    std::string load(const char*, uint32_t);
    std::string get_sdp_str() const;
    
    int set_receiver_ip(std::string& ip);
    int set_streamid(std::string& id);

  private:
    std::string generate_sdp_str();
    int add_media(rtp_media_info *s);
    int clear_media();
    void parse_sdp_str(const char *sdp_str, uint32_t len);
    int sdp_write_address(std::stringstream& ss, const char *dest_addr, const char *dest_type, int ttl);
    int sdp_write_header(std::stringstream& ss, sdp_session_level *s);
    int sdp_write_media(std::stringstream& ss, rtp_media_info *s);
    int sdp_write_media_attributes(std::stringstream& ss, rtp_media_info *s);
    char *extradata2psets(rtp_media_info *s, char* psets);
    char *extradata2config(rtp_media_info *s, char* config);
    uint8_t *avc_find_startcode(const uint8_t *p, const uint8_t *end);
    uint8_t *avc_find_startcode_internal(const uint8_t *p, const uint8_t *end);
    static char *base64_encode(char *out, int out_size, const uint8_t *in, int in_size);
    static int base64_decode(uint8_t *out, const char *in_str, int out_size);
    static char *data_to_hex(char *buff, const uint8_t *src, int s, int lowercase);
    static int hex_to_data(uint8_t *data, const char *p);
    static int strstart(const char *str, const char *pfx, const char **ptr);
    static void get_word(char *buf, int buf_size, const char **pp);
    static void get_word_until_chars(char *buf, int buf_size,
      const char *sep, const char **pp);
    static void get_word_sep(char *buf, int buf_size, const char *sep,
      const char **pp);
    static int isspace(int c);
    void parse_sdp_line(SDPParseState *s1, int letter, const char *buf);
    static int parse_h264_sdp_line(rtp_media_info *s, const char *attr, const char *value);
    static int parse_aac_sdp_line(rtp_media_info *s, const char *attr, const char *value);
    static int rtp_next_attr_and_value(const char **p, char *attr, int attr_size,
      char *value, int value_size);
    static int parse_fmtp(rtp_media_info *s, const char *line,
      int(*parse_fmtp_func)(rtp_media_info *s, const char *attr, const char *value));
    static int h264_parse_sprop_parameter_sets(rtp_media_info *s, uint8_t *data_ptr, uint32_t& size_ptr, const char *value);

  private:
    sdp_session_level* _sdp_header;
    std::vector<rtp_media_info*> _media_infos;
    std::stringstream _sdp_ss;
  };
}
