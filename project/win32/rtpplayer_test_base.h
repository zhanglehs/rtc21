#pragma once

#include <string>
#include <cstdint>

class RTPPlayerConfig {
public:
  RTPPlayerConfig();

  std::string lapi;
  std::string appid;
  std::string flv_name;
  std::string alias;
  std::string streamid;
  std::string download_ip;
  uint16_t download_http_port;
  std::string protocol;
  uint16_t download_udp_port;
  uint16_t download_tcp_port;
  std::string token;
  uint32_t buffer_len;

  uint32_t multi_players;    //default:1
  uint32_t player_by_ffmpeg;//default:false

  std::string enable_fec;
  std::string enable_nack;
  std::string reporturl;

  std::string config_file;
  bool tcp;

  //Broken screen
  uint32_t is_lostpacketStrategy;//
  uint32_t IlostpacketToScreen; // <= number of packet int I frame to screen
  uint32_t PlostpacketToScreen; // <= number of packet int P frame to screen

  uint32_t is_yuvDump;          // yuv dump
  uint32_t log_level;
  int      log_file_max_size;
  uint32_t lost_rate;
};

void load_player_config(int argc, char *argv[], RTPPlayerConfig &config);

#ifdef WIN32
int play_start(RTPPlayerConfig &config);
int play_stop();
void load_player_config(const char* config_file, RTPPlayerConfig &config);
#endif
