#ifndef LF_RTC_PLAYER_H
#define LF_RTC_PLAYER_H

#ifndef DLLEXPORT
#ifdef WIN32
#ifdef ENGINE_SDK_EXPORTS
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllimport)
#endif
#else
#define DLLEXPORT
#endif
#endif

#include "../engine_api/avengine_types.h"

class RtcPlayerInternal;

enum RtcPlayerErrorType {
  RTC_PLAYER_ERROR_CREATETHREAD_FAILED = 100,
  RTC_PLAYER_ERROR_INVALID_ALIAS,
  RTC_PLAYER_ERROR_NETWORK_TIMEOUT,
  RTC_PLAYER_ERROR_NETWORK_ERROR,
  RTC_PLAYER_ERROR_GETMCUINFO_FAILED,
  RTC_PLAYER_ERROR_GETSDP_FAILED,
  RTC_PLAYER_ERROR_MCU_DENY,
};

enum RtcPlayerStopType {
  RTC_PLAYER_STOP_BY_ERROR = 100,
  //RTC_PLAYER_STOP_BY_USER,
};

enum RtcPlayerState {
  RTC_PLAYER_STATE_UNSET = 100,
  RTC_PLAYER_STATE_INITIALIZING,
  RTC_PLAYER_STATE_RUNNING,
  // wParam中会带上失败的原因RtcPlayerErrorType。
  // 处于RTC_PLAYER_STATE_ERROR状态时内部会自动重试，重试成功后会发送RTC_PLAYER_STATE_RUNNING；
  // 如果确定不再重试了，会再次发送RTC_PLAYER_STATE_STOPPED消息
  RTC_PLAYER_STATE_ERROR,
  // wParam中会带上stop的原因RtcPlayerStopType
  // 用户自己调用Stop函数导致的stop并不会发送RTC_PLAYER_STATE_STOPPED回调通知
  RTC_PLAYER_STATE_STOPPED,
};

enum RtcPlayerMsgid {
  RTC_PLAYER_MSG_VIDEO_RESOLUTION = RTC_PLAYER_STATE_STOPPED + 100,
  RTC_PLAYER_MSG_VIDEO_FIRST_FRAME,
  RTC_PLAYER_MSG_VIDEO_SNAPSHOT,   // wParam: (const char*), path; lParam: (bool), is_success.
};

namespace live_stream_sdk {
  class RTPDownload;
}

class DLLEXPORT RtcPlayer {
public:
  // msgid: RtcPlayerMsgid + RtcPlayerState
  typedef void(*RtcNetworkCallback)(RtcPlayer* ctx, int msgid, long wParam, long lParam);

  typedef void(*RtcDecodedVideoCallback)(RtcPlayer* ctx, char* data[3], lfrtcRawVideoType type, int wd, int ht);

  struct NetworkConfig {
    char lapi[256];
    char appid[256];
    char alias[256];
    char token[256];
    NetworkConfig() {
      lapi[0] = 0;
      appid[0] = 0;
      alias[0] = 0;
      token[0] = 0;
    }
  };

public:
  RtcPlayer(const char *deviceid);
  ~RtcPlayer();

  int Start(const NetworkConfig* net, RtcNetworkCallback callback = 0);
  int Stop();
  int SetWindow(void *hwnd);
  int Snapshot(const char *path);
  void Mute(bool mute);
  void SetNetworkChanged();
  void SetDecodedVideoCallback(RtcDecodedVideoCallback callback);
  RtcPlayerState GetState();

  //int GetVideoFps();

  live_stream_sdk::RTPDownload *GetDownloader();

  void SetUserdata(void *userdata) { m_userdata = userdata; }
  void *GetUserdata() { return m_userdata; }

private:
  RtcPlayerInternal *m_internal;
  void* m_userdata;
};

#endif
