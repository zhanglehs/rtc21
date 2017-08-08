#include "engine_api/rtp_api.h"
#include "LFListener.h"
#include "engine_api/RtcLog.h"
#include "engine_api/RtcPlayer.h"
#include "myLog.h"

#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string>

#ifdef TAG
#undef TAG
#endif
#define TAG "LFRtpPlayer"
#define LFRTP_LOG_PATH "/sdcard/lfrtplog"

using namespace live_stream_sdk;

static const char* mClassPathName = "com/laifeng/rtpmediasdk/player/RtpPlayer";

static JavaVM* jVM = NULL;

namespace {
  class CJstringToString {
  public:
    CJstringToString(JNIEnv* env, jstring jstr) {
      if (jstr) {
        const char* str = env->GetStringUTFChars(jstr, JNI_FALSE);
        if (str) {
          m_str = str;
          env->ReleaseStringUTFChars(jstr, str);
        }
      }
    }

    const char* getString() {
      return m_str.c_str();
    }

  private:
    std::string m_str;
  };
}

static void player_message_callback(RtcPlayer* ctx, int msgid, long wParam, long lParam) {
  LFListener *listener = (LFListener *)ctx->GetUserdata();
  if (listener != NULL){
    listener->notifyMessage(msgid, wParam, lParam);
  }
}

static void
com_youku_lflive_LFLiveAPI_register(JNIEnv* env, jobject thiz, jobject context) {
  LOGI("com_youku_lflive_LFLiveAPI_register");
  rtcAndroidInit((void*)jVM, (void*)context);
}

static void
com_youku_lflive_LFLiveAPI_unregister(JNIEnv* env, jobject thiz) {
  LOGI("com_youku_lflive_LFLiveAPI_unregister");
  rtcAndroidUninit();
}

static jlong
com_youku_lflive_LFLiveAPI_CreatePlayer(JNIEnv* env, jobject thiz){
  INF("com_youku_lflive_LFLiveAPI_CreatePlayer");
  RtcPlayer *player = new RtcPlayer("xxxdeviceidxxx");
  player->SetUserdata(new LFListener(env, thiz));
  return (jlong)player;
}

static int com_youku_lflive_LFLiveAPI_SetOsVersion(JNIEnv* env, jobject thiz, jstring osVersion) {
  return 0;
}

static int
com_youku_lflive_LFLiveAPI_StartPlay(JNIEnv* env, jobject thiz, jlong playerCtx, jstring appid,
jstring alias, jstring host, jstring token, jstring deviceid, jint logLevel){
  LOGI("com_youku_lflive_LFLiveAPI_StartPlay");
  mkdir(LFRTP_LOG_PATH, 0777);
  if (logLevel == 1)
    live_stream_sdk::LogSetLevel(live_stream_sdk::LOG_LEVEL_NON);
  else if (logLevel == 2)
    live_stream_sdk::LogSetLevel(live_stream_sdk::LOG_LEVEL_DBG);
  live_stream_sdk::LogSetDir(LFRTP_LOG_PATH, NULL);

  CJstringToString app_id(env, appid);
  CJstringToString alias_id(env, alias);
  CJstringToString host_id(env, host);
  CJstringToString token_id(env, token);
  RtcPlayer::NetworkConfig config;
  strcpy(config.alias, alias_id.getString());
  strcpy(config.appid, app_id.getString());
  strcpy(config.lapi, host_id.getString());
  strcpy(config.token, token_id.getString());
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  return player->Start(&config, player_message_callback);
}

static int
com_youku_lflive_LFLiveAPI_SetWindow(JNIEnv* env, jobject thiz, jlong playerCtx, jobject gl_surface0) {
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  player->SetWindow(gl_surface0);
}

static int
com_youku_lflive_LFLiveAPI_SetNetworkChanged(JNIEnv* env, jobject thiz, jlong playerCtx){
  LOGI("com_youku_lflive_LFLiveAPI_SetNetworkChanged");
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  player->SetNetworkChanged();
  return 0;
}

static int
com_youku_lflive_LFLiveAPI_StopPlay(JNIEnv* env, jobject thiz, jlong playerCtx){
  LOGI("com_youku_lflive_LFLiveAPI_StopPlay");
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  player->Stop();
  return 0;
}

static int
com_youku_lflive_LFLiveAPI_DestroyPlayer(JNIEnv* env, jobject thiz, jlong playerCtx){
  LOGI("com_youku_lflive_LFLiveAPI_DestroyPlayer");
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  LFListener *listener = (LFListener *)player->GetUserdata();
  delete player;
  delete listener;
  return 0;
}

static int
com_youku_lflive_LFLiveAPI_SetAVMute(JNIEnv* env, jobject thiz, jlong playerCtx, jboolean mute){
  LOGI("com_youku_lflive_LFLiveAPI_SetAVMute");
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  player->Mute((bool)mute);
  return 0;
}

static bool
com_youku_lflive_LFLiveAPI_snapShot(JNIEnv* env, jobject thiz, jlong playerCtx, jstring path){
  LOGI("com_youku_lflive_LFLiveAPI_snapShot");
  CJstringToString _path(env, path);
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  return player->Snapshot(_path.getString());
}

static JNINativeMethod mMethods[] = {//method for JAVA. use this to register native method
  { "native_register", "(Ljava/lang/Object;)V", (void*)com_youku_lflive_LFLiveAPI_register },
  { "native_unregister", "()V", (void*)com_youku_lflive_LFLiveAPI_unregister },
  { "native_CreatePlayer", "()J", (void*)com_youku_lflive_LFLiveAPI_CreatePlayer },
  { "native_StartPlay", "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)I", (void*)com_youku_lflive_LFLiveAPI_StartPlay },
  { "native_SetWindow", "(JLjava/lang/Object;)I", (void*)com_youku_lflive_LFLiveAPI_SetWindow },
  { "native_SetNetworkChanged", "(J)I", (void*)com_youku_lflive_LFLiveAPI_SetNetworkChanged },
  { "native_StopPlay", "(J)I", (void*)com_youku_lflive_LFLiveAPI_StopPlay },
  { "native_DestroyPlayer", "(J)I", (void*)com_youku_lflive_LFLiveAPI_DestroyPlayer },
  { "native_SetAVMute", "(JZ)I", (void*)com_youku_lflive_LFLiveAPI_SetAVMute },
  { "native_SetOsVersion", "(Ljava/lang/String;)I", (void *)com_youku_lflive_LFLiveAPI_SetOsVersion },
  { "native_snapShot", "(JLjava/lang/String;)Z", (void *)com_youku_lflive_LFLiveAPI_snapShot },
};

static int jniRegisterNativeMethods(JNIEnv* env, const JNINativeMethod* methods, int count) {
  jclass clazz;
  int ret = -1;
  clazz = env->FindClass(mClassPathName);
  if (clazz == NULL) {
    return ret;
  }
  if ((ret = env->RegisterNatives(clazz, methods, count)) < 0) {
    return ret;
  }
  return ret;
}

int registerLFLiveAPIMethods(JNIEnv* env) {
  JavaVM* jVM;
  env->GetJavaVM((JavaVM**)&jVM);
  return jniRegisterNativeMethods(env, mMethods, sizeof(mMethods) / sizeof(mMethods[0]));
}

extern int registerRtpUploadMethods(JNIEnv* env);

//JNI main
int JNI_OnLoad(JavaVM* vm, void* reserved) {
  jVM = vm;
  JNIEnv* env = NULL;
  jint ret = JNI_ERR;
  if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
    LOGE("GetEnv failed!");
    return ret;
  }
  assert(env != NULL);
  if (registerLFLiveAPIMethods(env) != JNI_OK) {
    LOGE("can not load lfliveapi methods!");
    return ret;
  }
  if (registerRtpUploadMethods(env) != JNI_OK) {
    LOGE("can not load rtpupload methods!");
    return ret;
  }
  ret = JNI_VERSION_1_4;
  LOGI("Loaded!");
  return ret;
}

