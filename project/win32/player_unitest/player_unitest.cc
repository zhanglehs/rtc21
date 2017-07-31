#include "../rtpplayer_test_base.h"

#include "engine_api/RtcLog.h"
#include <windows.h>

int main(int argc, char *argv[]) {
  live_stream_sdk::LogSetDir("D:\\rtp_test");

  RTPPlayerConfig config;
  load_player_config(argc, argv, config);
  play_start(config);

  while (true) {
    Sleep(100);
  }

  play_stop();
  system("pause");
  return 1;
}
