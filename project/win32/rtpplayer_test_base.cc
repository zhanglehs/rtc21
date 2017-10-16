#include "rtpplayer_test_base.h"

#include "engine_api/RtcLog.h"
#include "engine_api/rtp_api.h"
#include "engine_api/RtcPlayer.h"
#include "engine_api/rtp_download.h"
#include "getopt.h"
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <windows.h>
#include <atlbase.h>
#include <atlwin.h>

#ifdef WIN32
#include <Windows.h>
#include <Iphlpapi.h>
#pragma warning(disable: 4996)
#pragma warning(disable: 4018)
#endif

#pragma comment(lib, "getopt.lib")

static void display_help() {
  printf("  --version\t\t display version\n");
  printf("  --help\t\t display this help\n");
  printf("stream options:\n");
  printf("  -l, --lpai <lapi_url>\t\t lapi url, default lapi.xiu.youku.com:80\n");
  printf("  -a, --alias <alias>\t\t find stream by alias, this option will overwrite streamid option\n");
  printf("  -s, --streamid <streamid>\t\t find stream by streamid(32 Bytes string)\n");
  printf("  -d, --download_ip <ip>\t\t stream server ip\n");
  printf("  -r, --reporturl <ip>\t\t stream server reporturl\n");
  printf("  -H, --http_port <port>\t\t stream server http port\n");
  printf("  -p, --protocol <tcp, udp>\t\t stream server network protocol, default udp\n");
  printf("  -t, --tcp_port <port>\t\t stream server tcp port\n");
  printf("  -u, --udp_port <port>\t\t stream server udp port\n");
  printf("  -k, --token <token>\t\t super token\n");
  printf("  -f, --player_by_ffmpeg <bool>\t\t player_by_ffmpeg 1 avengine 0\n");
  printf("  -m, --multi_players  number of the palyers default 1\n");
  printf("  -L： 0 not use lostpacketStrategy, 1 use lostpacketStrategy default： 1 ");
  printf("  -I： number of packet int I frame to screen  default 0");
  printf("  -P： number of packet int P frame to screen  default 0");
  printf("  -D： dump yuv  default 0");
  printf("  -g： loglevel  default 6");
  printf("  -S： log_file_max_size  default 10  0 not slplit  1 1M");
  printf("  -z： lostrate  default 0");
  printf("  -i,  input filename\n");
  printf("  -A,  appid\n");
}


RTPPlayerConfig::RTPPlayerConfig()
:download_http_port(0),
download_udp_port(0),
download_tcp_port(0),
player_by_ffmpeg(0),
multi_players(1),
is_lostpacketStrategy(1),
IlostpacketToScreen(0),
PlostpacketToScreen(0),
is_yuvDump(0),
log_level(0),
log_file_max_size(1),
lost_rate(0) {
}

void load_player_config(int argc, char *argv[], RTPPlayerConfig &config) {
  static struct option_a long_options[] = {
    { "version", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { "lapi_url", required_argument, NULL, 'l' },
    { "appid", required_argument, NULL, 'A' },
    { "flv_name", required_argument, NULL, 'i' },
    { "alias", required_argument, NULL, 'a' },
    { "streamid", required_argument, NULL, 's' },
    { "download_ip", required_argument, NULL, 'd' },
    { "reporturl", required_argument, NULL, 'r' },
    { "http_port", required_argument, NULL, 'H' },
    { "protocol", required_argument, NULL, 'p' },
    { "tcp_port", required_argument, NULL, 't' },
    { "udp_port", required_argument, NULL, 'u' },
    { "token", required_argument, NULL, 'k' },
    { "fec", required_argument, NULL, 'F' },
    { "nack", required_argument, NULL, 'n' },
    { "buffer_len", required_argument, NULL, 'b' },
    { "player_by_ffmpeg", required_argument, NULL, 'f' },
    { "config_file", required_argument, NULL, 'c' },
    { "multi_players", required_argument, NULL, 'm' },
    { "is_lostpacketStrategy", required_argument, NULL, 'L' },
    { "IlostpacketToScreen", required_argument, NULL, 'I' },
    { "PlostpacketToScreen", required_argument, NULL, 'P' },
    { "DumpYUV", required_argument, NULL, 'D' },
    { "log_level", required_argument, NULL, 'g' },
    { "log_file_max_size", required_argument, NULL, 'S' },
    { "lost_rate", required_argument, NULL, 'z' },
    { 0, 0, 0, 0 }
  };

  int opt;
  int option_index = 0;
  char* optstring = "hl:a:s:d:H:p:t:u:k:b:f:F:c:m:n:L:I:P:D:G:g:S:r:z:v:i:A:"; //There must be an ":" in the end of string
  std::string temp;

  while ((opt = getopt_long_a(argc, argv, optstring, long_options, &option_index)) != -1)
  {
    switch (opt)
    {
    case 'h':
      display_help();
      exit(0);
    case 'l':
      config.lapi = optarg_a;
      break;
    case 'a':
      config.alias = optarg_a;
      break;
    case 's':
      config.streamid = optarg_a;
      break;
    case 'd':
      config.download_ip = optarg_a;
      break;
    case 'r':
      config.reporturl = optarg_a;
      break;
    case 'H':
      config.download_http_port = atoi(optarg_a);
      break;
    case 'p':
      config.protocol = optarg_a;
      break;
    case 't':
      config.download_tcp_port = atoi(optarg_a);
      break;
    case 'u':
      config.download_udp_port = atoi(optarg_a);
      break;
    case 'k':
      config.token = optarg_a;
      break;
    case 'b':
      config.buffer_len = atoi(optarg_a);
      break;
    case 'f':
      config.player_by_ffmpeg = atoi(optarg_a);
      break;
    case 'm':
      config.multi_players = atoi(optarg_a);
      break;
    case 'F':
      temp = optarg_a;
      std::transform(temp.begin(), temp.end(), temp.begin(), tolower);
      config.enable_fec = temp;
      break;
    case 'n':
      temp = optarg_a;
      std::transform(temp.begin(), temp.end(), temp.begin(), tolower);
      config.enable_nack = temp;
      break;
    case 'c':
      config.config_file = optarg_a;
      break;
    case 'L':
      config.is_lostpacketStrategy = atoi(optarg_a);
      break;
    case 'I':
      config.IlostpacketToScreen = atoi(optarg_a);
      break;
    case 'P':
      config.PlostpacketToScreen = atoi(optarg_a);
      break;
    case 'D':
      config.is_yuvDump = atoi(optarg_a);
      break;
    case 'g':
      config.log_level = atoi(optarg_a);
      break;
    case 'S':
      config.log_file_max_size = atoi(optarg_a);
      break;
    case 'z':
      config.lost_rate = atoi(optarg_a);
      break;
    case 'i':
      config.flv_name = optarg_a;
      break;
    case 'A':
      config.appid = optarg_a;
      break;
    }
  }
}

#ifdef WIN32

void load_player_config(const char* config_file, RTPPlayerConfig &config) {
  char lapi[64];
  GetPrivateProfileStringA("player", "lapi", "101.201.57.242", lapi, _countof(lapi) - 1, config_file);
  config.lapi = lapi;
  char appid[64];
  GetPrivateProfileStringA("player", "appid", "301", appid, _countof(appid) - 1, config_file);
  config.appid = appid;
  char alias[64];
  GetPrivateProfileStringA("player", "alias", "", alias, _countof(alias) - 1, config_file);
  config.alias = alias;
  char receiver_ip[64];
  GetPrivateProfileStringA("player", "receiver_ip", "", receiver_ip, _countof(receiver_ip) - 1, config_file);
  config.download_ip = receiver_ip;
  char receiver_udp_port[64];
  GetPrivateProfileStringA("player", "receiver_udp_port", "", receiver_udp_port, _countof(receiver_udp_port) - 1, config_file);
  config.download_udp_port = (uint16_t)atoi(receiver_udp_port);
  char receiver_tcp_port[64];
  GetPrivateProfileStringA("player", "receiver_tcp_port", "", receiver_tcp_port, _countof(receiver_tcp_port) - 1, config_file);
  config.download_tcp_port = (uint16_t)atoi(receiver_tcp_port);
  char receiver_http_port[64];
  GetPrivateProfileStringA("player", "receiver_http_port", "", receiver_http_port, _countof(receiver_http_port) - 1, config_file);
  config.download_http_port = (uint16_t)atoi(receiver_http_port);
  char player_count[64];
  GetPrivateProfileStringA("player", "player_count", "1", player_count, _countof(player_count) - 1, config_file);
  config.multi_players = (uint32_t)atoi(player_count);
  char streamid[64];
  GetPrivateProfileStringA("player", "streamid", "", streamid, _countof(streamid) - 1, config_file);
  config.streamid = streamid;
  config.tcp = !!GetPrivateProfileIntA("player", "is_tcp", 0, config_file);
  // TODO: 待续
}

namespace {

using namespace live_stream_sdk;

void OnMsgCallback(RtcPlayer* ctx, int msgid, long wParam, long lParam) {
  int i = 0;
  i++;
#ifdef WIN32
  char msg[128];
  sprintf(msg, ".......................player callback, msgid=%u, wparam=%d, lparam=%d\n", msgid, wParam, lParam);
  OutputDebugStringA(msg);
#endif
}

void OnGetVideoRawDataCallback(RtcPlayer* ctx, char* data[3], lfrtcRawVideoType type, int wd, int ht) {
  int i = 0;
  i++;
}

void PrintMACaddress(unsigned char MACData[],char *mac) {
  sprintf(mac,"%02X-%02X-%02X-%02X-%02X-%02X",
    MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
}

void GetMacAddress(char *mac) {
  IP_ADAPTER_INFO AdapterInfo[16];
  DWORD dwBuflen = sizeof(AdapterInfo);
  DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBuflen);
  PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
  do{
    PrintMACaddress(pAdapterInfo->Address,mac);
    pAdapterInfo = pAdapterInfo->Next;
  } while (0);//while (pAdapterInfo);
}

class CPlayerWindow : public CWindowImpl<CPlayerWindow, CWindow, CWinTraits<WS_OVERLAPPEDWINDOW, 0> >  {
public:
  DECLARE_WND_CLASS(L"CPlayerWindow")

  BEGIN_MSG_MAP(CPlayerWindow)
    //MESSAGE_HANDLER(WM_PAINT, OnPaint)
  END_MSG_MAP()

  CPlayerWindow() {
    RECT rc = { 0, 0, 360, 640 };
    Create(NULL, &rc, L"player_window");
    ShowWindow(SW_SHOW);
  }
};

std::vector<RtcPlayer*> s_players;

}

int play_start(RTPPlayerConfig &config) {
  play_stop();

  if (config.log_level > 0) {
    LogSetLevel((LogLevel)config.log_level);
  }

  if (config.multi_players < 0 || config.multi_players > 10) {
    return -1;
  }

  //for avengine params
  lfrtcGlobalConfig* p_avengine = getAvengineParamsSDK();
  p_avengine->is_lostpacketStrategy = config.is_lostpacketStrategy;
  p_avengine->IlostpacketToScreen = config.IlostpacketToScreen;
  p_avengine->PlostpacketToScreen = config.PlostpacketToScreen;
  p_avengine->is_yuvDump = config.is_yuvDump;

  char macaddress[1024];
  macaddress[0] = '\0';
  GetMacAddress(macaddress);
  for (int i = 0; i < config.multi_players; i++) {
    RtcPlayer *player = new RtcPlayer(macaddress);
    player->SetUserdata(new CPlayerWindow());
    s_players.push_back(player);
  }

  bool auto_schedule = true;
  if (config.streamid.length() && config.download_ip.length()) {
    auto_schedule = false;
  }

  char appid[1024];
  char token[1024];
  char lapi[1024];
  if (config.token.length() > 0) {
    strcpy(token, config.token.c_str());
  }
  else {
    strcpy(token, "98765");
  }
  if (config.appid.length() > 0) {
    strcpy(appid, config.appid.c_str());
  }
  else {
    strcpy(appid, "301");
  }
  if (config.lapi.length() > 0) {
    strcpy(lapi, config.lapi.c_str());
  }
  else {
    //strcpy(lapi, "101.200.47.145");
    //strcpy(lapi, "103.41.143.105");
    strcpy(lapi, "101.201.57.242");
  }

  RtcPlayer::NetworkConfig player_config;
  strcpy(player_config.lapi, lapi);
  strcpy(player_config.appid, appid);
  strcpy(player_config.alias, config.alias.c_str());
  strcpy(player_config.token, token);
  for (int i = 0; i < config.multi_players; i++) {
    s_players[i]->Start(&player_config, OnMsgCallback);

    //if (auto_schedule && config.download_ip.length()) {
    //  // 获取streamid，然后再使用指定的download信息拉流
    //  while (true) {
    //    rtcDownloadDispatchConfig dispatch;
    //    s_players[i]->GetDownloader()->GetDispatchConfig(&dispatch);
    //    if (dispatch.mcu_udp_port) {
    //      Sleep(100);
    //      config.streamid = dispatch.streamid;
    //      break;
    //    }
    //    Sleep(10);
    //  }
    //}

    if (config.download_ip.length()) {
      rtcDownloadDispatchConfig dispatch_config;
      strcpy(dispatch_config.appid, appid);
      strcpy(dispatch_config.alias, config.alias.c_str());
      strcpy(dispatch_config.lapi_host, lapi);
      strcpy(dispatch_config.lapi_token, token);
      dispatch_config.disable_playlist = !auto_schedule;

      strcpy(dispatch_config.streamid, config.streamid.c_str());
      strcpy(dispatch_config.mcu_ip, config.download_ip.c_str());
      dispatch_config.mcu_udp_port = config.download_udp_port;
      dispatch_config.mcu_tcp_port = config.download_tcp_port;
      dispatch_config.is_tcp = config.tcp;
      strcpy(dispatch_config.mcu_token, "98765");
      if (dispatch_config.mcu_ip[0]) {
        sprintf(dispatch_config.sdp_url, "http://%s:%d/download/sdp?streamid=%s",
          config.download_ip.c_str(), config.download_http_port, config.streamid.c_str());
      }

      s_players[i]->GetDownloader()->Stop();
      s_players[i]->GetDownloader()->Start(&dispatch_config);
    }

    s_players[i]->SetWindow(((CPlayerWindow*)(s_players[i]->GetUserdata()))->m_hWnd);
    s_players[i]->GetDownloader()->DebugLost(config.lost_rate, config.lost_rate);
    s_players[i]->SetDecodedVideoCallback(OnGetVideoRawDataCallback);
  }

  return 1;
}

int play_stop() {
  for (auto it = s_players.begin(); it != s_players.end(); it++) {
    RtcPlayer *player = *it;
    CPlayerWindow *window = (CPlayerWindow*)(player->GetUserdata());
    delete player;
    if (window->IsWindow()) {
      window->DestroyWindow();
    }
    delete window;
  }
  s_players.clear();
  return 0;
}

#endif
