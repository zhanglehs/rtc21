#include "engine_api/rtp_api.h"
#include "LFListener.h"
#include "engine_api/RtcLog.h"
#include "engine_api/RtcPlayer.h"
#include "myLog.h"

#include <stdio.h>
#include <jni.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
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
com_youku_lflive_LFLiveAPI_setup(JNIEnv* env, jobject thiz) {
}

static void
com_youku_lflive_LFLiveAPI_register(JNIEnv* env, jobject thiz, jobject context)
{
  LOGI("com_youku_lflive_LFLiveAPI_register");
  rtcAndroidInit((void*)jVM, (void*)context);
}

static void
com_youku_lflive_LFLiveAPI_unregister(JNIEnv* env, jobject thiz)
{
  LOGI("com_youku_lflive_LFLiveAPI_unregister");
  rtcAndroidUninit();
}

static jlong
com_youku_lflive_LFLiveAPI_CreatePlayer(JNIEnv* env, jobject thiz){
  LOGI("com_youku_lflive_LFLiveAPI_CreatePlayer");
  INF("com_youku_lflive_LFLiveAPI_CreatePlayer");

  RtcPlayer *player = new RtcPlayer("xxxdeviceidxxx");
  player->SetUserdata(new LFListener(env, thiz));
  return (jlong)player;
}

static void
com_youku_lflive_LFLiveAPI_SetDownloadParams(JNIEnv* env, jobject thiz, jlong playerCtx,
jstring downloadip, jstring downloadudpport, jstring downloadtcpport, jstring downloadhttpport,
jstring downloadstreamid, jint enableFec, jint enableNack, jint is_lostpacketStrategy,
jint IlostpacketToScreen, jint PlostpacketToScreen){
  //LOGI("com_youku_lflive_LFLiveAPI_SetDownloadParams");

  //CJstringToString download_ip(env,downloadip);
  //CJstringToString download_udpport(env,downloadudpport);
  //CJstringToString download_tcpport(env,downloadtcpport);
  //CJstringToString download_httpport(env,downloadhttpport);
  //CJstringToString download_streamid(env,downloadstreamid);

  //LOGI("download_ip:%s download_udpport:%s  download_tcpport:%s download_httpport:%s download streamid:%s,enableFec:%d,enableNack:%d",
  //	 download_ip.getString(),download_udpport.getString(),download_tcpport.getString(),
  //	 download_httpport.getString(),download_streamid.getString(),enableFec,enableNack);
  //       rtcAdvancedSetDownloadIp((void *)playerCtx,download_ip.getString());
  //       rtcAdvancedSetDownloadUDPPort((void *)playerCtx,atoi(download_udpport.getString()));
  //       rtcAdvancedSetDownloadTCPPort((void *)playerCtx,atoi(download_tcpport.getString()));
  //       rtcAdvancedSetDownloadToken((void *)playerCtx,"98765");
  //       rtcAdvancedSetStreamId((void *)playerCtx,download_streamid.getString());
  //       rtcAdvancedSetEnableNACK((void *)playerCtx,enableNack);
  //       AVENGINE_PARAMS_SDK* pAvParams = getAvengineParamsSDK();
  //       pAvParams->is_lostpacketStrategy = is_lostpacketStrategy;
  //       pAvParams->IlostpacketToScreen = IlostpacketToScreen;
  //       pAvParams->PlostpacketToScreen = PlostpacketToScreen;
  //       char sdpurl[1024];
  //       sprintf(sdpurl,"http://%s:%s/download/sdp/%s?token=98765",
  //			download_ip.getString(),download_httpport.getString(),download_streamid.getString());
  //       LOGI("download sdp url:%s",sdpurl);
  //       rtcAdvancedSetDownloadSDPUrl((void *)playerCtx,sdpurl);
}

jstring com_youku_lflive_LFLiveAPI_GetDownloadIp(JNIEnv* env, jobject thiz, jlong playerCtx)
{
  return env->NewStringUTF("");
  //const char *ipaddr = rtcAdvancedGetDownloadIp((void *)playerCtx);
  //if(ipaddr == NULL)
  //    return env->NewStringUTF("");
  //jstring ip = env->NewStringUTF(ipaddr);
  //LOGI("get download ip:%s",ipaddr);
  //return ip;
}

jstring com_youku_lflive_LFLiveAPI_native_getPlaylist(JNIEnv* env, jobject thiz, jlong playerCtx)
{
  return env->NewStringUTF("");
  //const char *playlist = rtcGetPlaylist((void *)playerCtx);
  //if(playlist == NULL)
  //    return env->NewStringUTF("");
  //jstring str = env->NewStringUTF(playlist);
  //LOGI("get playlist:%s",playlist);
  //return str;
}

static int com_youku_lflive_LFLiveAPI_SetOsVersion(JNIEnv* env, jobject thiz, jstring osVersion)
{
  const char *c_osVersion = env->GetStringUTFChars(osVersion, NULL);
  // live_stream_sdk::LogSetOsVersion(c_osVersion);
  env->ReleaseStringUTFChars(osVersion, c_osVersion);
  return 0;
}

int com_youku_lflive_LFLiveAPI_GetDownloadUdpPort(JNIEnv* env, jobject thiz, jlong playerCtx)
{
  //int udpport = rtcAdvancedGetDownloadUDPPort((void *)playerCtx);
  //LOGI("get download udp port:%d",udpport);
  //return udpport;
  return 0;
}

int com_youku_lflive_LFLiveAPI_GetDownloadTcpPort(JNIEnv* env, jobject thiz, jlong playerCtx)
{
  //int tcpport = rtcAdvancedGetDownloadTCPPort((void *)playerCtx);
  //LOGI("get download tcp port:%d",tcpport);
  //return tcpport;
  return 0;
}

int com_youku_lflive_LFLiveAPI_GetDownloadHttpPort(JNIEnv* env, jobject thiz, jlong playerCtx)
{
  //const char * sdpurl = rtcAdvancedGetDownloadSDPUrl((void *)playerCtx);
  //char port[10] = {0};
  //char *pStart = strrchr(sdpurl,':');
  //if(pStart == NULL)
  //    return 80;
  //else {
  //    char *pEnd = pStart;
  //    while(pEnd && *pEnd !='/')
  //        pEnd++;
  //    if(pEnd) {
  //        memcpy(port,pStart + 1,pEnd - pStart -1);
  //        LOGI("http port:%s",port);
  //    }
  //}

  //return atoi(port);
  return 0;
}

jstring com_youku_lflive_LFLiveAPI_GetDownloadStreamId(JNIEnv* env, jobject thiz, jlong playerCtx)
{
  return env->NewStringUTF("");
  //const char *pid = rtcAdvancedGetStreamId((void *)playerCtx);
  //if(pid == NULL)
  //    return env->NewStringUTF("");
  //jstring streamid = env->NewStringUTF(pid);
  //LOGI("get download streamid:%s",pid);
  //return streamid;
}

//static void log_to_logcat(int level, const char *file, int line_num, const char *format, va_list args)
//{
//  char buf[1024 * 16];
//  buf[0] = 0;
//  live_stream_sdk::get_log_str(buf, sizeof(buf), level, file, line_num, format, args);
//
//  switch (level)
//  {
//  case live_stream_sdk::LOG_LEVEL_TRC:
//  {
//                                       LOGV("%s", buf);
//                                       break;
//  }
//  case live_stream_sdk::LOG_LEVEL_DBG:
//  {
//                                       LOGD("%s", buf);
//                                       break;
//  }
//  case live_stream_sdk::LOG_LEVEL_INF:
//  {
//                                       LOGI("%s", buf);
//                                       break;
//  }
//  case live_stream_sdk::LOG_LEVEL_WRN:
//  {
//                                       LOGW("%s", buf);
//                                       break;
//  }
//  case live_stream_sdk::LOG_LEVEL_ERR:
//  {
//                                       LOGE("%s", buf);
//                                       break;
//  }
//  }
//}

static void RtcLogCallback(const char *msg) {
  LOGI("%s", msg);
}

static int
com_youku_lflive_LFLiveAPI_StartPlay(JNIEnv* env, jobject thiz, jlong playerCtx, jstring appid,
jstring alias, jstring host, jstring token, jstring deviceid, jstring extraparams, jint logLevel){
  mkdir(LFRTP_LOG_PATH, 0777);
  if (logLevel == 1)
    live_stream_sdk::LogSetLevel(live_stream_sdk::LOG_LEVEL_NON);
  else if (logLevel == 2)
    live_stream_sdk::LogSetLevel(live_stream_sdk::LOG_LEVEL_DBG);
  live_stream_sdk::LogSetDir(LFRTP_LOG_PATH, RtcLogCallback);

  LOGI("com_youku_lflive_LFLiveAPI_StartPlay");
  RtcPlayer *player = (RtcPlayer*)playerCtx;

  CJstringToString app_id(env, appid);
  CJstringToString alias_id(env, alias);
  CJstringToString host_id(env, host);
  CJstringToString token_id(env, token);
  RtcPlayer::NetworkConfig config;
  strcpy(config.alias, alias_id.getString());
  strcpy(config.appid, app_id.getString());
  strcpy(config.lapi, host_id.getString());
  strcpy(config.token, token_id.getString());
  return player->Start(&config, player_message_callback);



  //	CJstringToString device_id(env,deviceid);
  //	CJstringToString _extraparams(env,extraparams);
  ////        if (strcmp(host_id,"101.201.57.242") == 0) {
  ////            char repurl[128] = {0};
  ////            strcpy(repurl,host_id);
  ////            strcat(repurl,":6601");
  ////            live_stream_sdk::LiveStreamSDKSetReportUrl(repurl,"/v1/upload_file");
  ////        }
  //	LOGI("appid:%s alias:%s host:%s token:%s", app_id.getString(),
  //		 alias_id.getString(), host_id.getString(),token_id.getString());
  //    LOGI("laifeng-debug StartPlay:%lld", playerCtx);
  //
  //	int ret = rtcStartPlay((void *)playerCtx, app_id.getString(), alias_id.getString(),
  //		 host_id.getString(), token_id.getString(), device_id.getString(), _extraparams.getString());
  //	INF("com_youku_lflive_LFLiveAPI_StartPlay %lld ret:%d ---", playerCtx,ret);
  //	return ret;
}

static int
com_youku_lflive_LFLiveAPI_SetWindow(JNIEnv* env, jobject thiz, jlong playerCtx, jobject gl_surface0) {
  //return rtcSetPlayerWindow((void *)playerCtx, gl_surface0);
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  player->SetWindow(gl_surface0);
}

static int
com_youku_lflive_LFLiveAPI_AdvancedInitPlayer(JNIEnv* env, jobject thiz,
jlong playerCtx, jstring appid, jstring alias, jstring host, jstring token,
jstring deviceid, jint logLevel){
  return 0;
  //	mkdir(LFRTP_LOG_PATH,0777);
  //	if(logLevel == 1)
  //		live_stream_sdk::LogSetLevel(live_stream_sdk::LOG_LEVEL_NON);
  //	else if(logLevel == 2)
  //		live_stream_sdk::LogSetLevel(live_stream_sdk::LOG_LEVEL_DBG);
  //	live_stream_sdk::LogSetCallback(log_to_logcat,LFRTP_LOG_PATH,live_stream_sdk::PlayerType::DOWNLOADER);
  //
  //	LOGI("com_youku_lflive_LFLiveAPI_AdvancedInitPlay");
  //	CJstringToString app_id(env,appid);
  //	CJstringToString alias_id(env,alias);
  //	CJstringToString host_id(env,host);
  //	CJstringToString token_id(env,token);
  //	CJstringToString device_id(env,deviceid);
  //
  //	LOGI("appid:%s alias:%s host:%s token:%s", app_id.getString(),
  //		 alias_id.getString(), host_id.getString(),token_id.getString());
  ////        if (strcmp(host_id,"101.201.57.242") == 0) {
  ////            char repurl[128] = {0};
  ////            strcpy(repurl,host_id);
  ////            strcat(repurl,":6601");
  ////            live_stream_sdk::LiveStreamSDKSetReportUrl(repurl,"/v1/upload_file");
  ////        }
  //	return rtcAdvancedInitPlayer((void *)playerCtx, app_id.getString(), alias_id.getString(),
  //								 host_id.getString(),token_id.getString(),device_id.getString());
}

static int
com_youku_lflive_LFLiveAPI_SetNetworkChanged(JNIEnv* env, jobject thiz, jlong playerCtx){
  LOGI("com_youku_lflive_LFLiveAPI_SetNetworkChanged");
  //return rtcSetNetworkChanged((void *)playerCtx);
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  player->SetNetworkChanged();
  return 0;
}

static int
com_youku_lflive_LFLiveAPI_AdvancedSetDownloadReportPlayDelay(JNIEnv* env, jobject thiz, jlong playerCtx, jboolean enable){
  LOGI("com_youku_lflive_LFLiveAPI_SetNetworkChanged");
  //return rtcAdvancedSetDownloadReportPlayDelay((void *)playerCtx,enable);
  return 0;
}

static int
com_youku_lflive_LFLiveAPI_AdvancedStartPlay(JNIEnv* env, jobject thiz, jlong playerCtx){
  LOGI("com_youku_lflive_LFLiveAPI_AdvancedStartPlay");
  //return rtcAdvancedStartPlay((void *)playerCtx);
  return 0;
}

static int
com_youku_lflive_LFLiveAPI_StopPlay(JNIEnv* env, jobject thiz, jlong playerCtx){
  LOGI("com_youku_lflive_LFLiveAPI_StopPlay");
  //INF("com_youku_lflive_LFLiveAPI_StopPlay %lld +++", playerCtx);
  //LOGI("laifeng-debug StopPlay:%lld", playerCtx);
  //rtcStopPlay((void *)playerCtx);
  //INF("com_youku_lflive_LFLiveAPI_StopPlay %lld ---", playerCtx);
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  //player->stop();
  return 0;
}

static int
com_youku_lflive_LFLiveAPI_DestroyPlayer(JNIEnv* env, jobject thiz, jlong playerCtx){
  LOGI("com_youku_lflive_LFLiveAPI_DestroyPlayer");
  //INF("com_youku_lflive_LFLiveAPI_DestroyPlayer %lld +++", playerCtx);
  //LOGI("laifeng-debug DestroyPlayer:%lld", playerCtx);
  //   LFListener *listener = (LFListener *)rtcGetPlayerUserdata((void *)playerCtx);
  //   rtcSetPlayerUserdata((void *)playerCtx, NULL);
  //rtcStopPlay((void *)playerCtx);
  //int ret = rtcDestroyPlayer((void *)playerCtx);
  //   if (listener != NULL){
  //       delete listener;
  //   }
  //INF("com_youku_lflive_LFLiveAPI_DestroyPlayer %lld ---", playerCtx);
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  LFListener *listener = (LFListener *)player->GetUserdata();
  delete player;
  delete listener;
  return 0;
}

static int
com_youku_lflive_LFLiveAPI_SetAVMute(JNIEnv* env, jobject thiz, jlong playerCtx, jint status){
  LOGI("com_youku_lflive_LFLiveAPI_SetAVMute");
  //return rtcSetAVMute((void *)playerCtx,status);
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  //player->Mute();
  return 0;
}

static int
com_youku_lflive_LFLiveAPI_SetAVMode(JNIEnv* env, jobject thiz, jlong playerCtx, jint av_mode){
  LOGI("com_youku_lflive_LFLiveAPI_SetAVMode");
  //return rtcSetAVModeNet((void *)playerCtx, av_mode);
  return 0;
}

static float
com_youku_lflive_LFLiveAPI_getFramesPerSecond(JNIEnv* env, jobject thiz, jlong playerCtx){
  LOGI("com_youku_lflive_LFLiveAPI_getFramesPerSecond");
  //return rtcGetVideoFPS((void *)playerCtx);
  return 0;
}

static long
com_youku_lflive_LFLiveAPI_getBitRate(JNIEnv* env, jobject thiz, jlong playerCtx){
  LOGI("com_youku_lflive_LFLiveAPI_getBitRate");
  //return rtcGetVideoBitRate((void *)playerCtx);
  return 0;
}

static long
com_youku_lflive_LFLiveAPI_getAvgKeyFrameSize(JNIEnv* env, jobject thiz, jlong playerCtx){
  LOGI("com_youku_lflive_LFLiveAPI_getAvgKeyFrameSize");
  //return rtcGetAvgKeyFrameSize((void *)playerCtx);
  return 0;
}

static bool
com_youku_lflive_LFLiveAPI_snapShot(JNIEnv* env, jobject thiz, jlong playerCtx, jstring path){
  LOGI("com_youku_lflive_LFLiveAPI_snapShot");
  CJstringToString _path(env, path);
  //return rtcSnapShot((void *)playerCtx,_path.getString());
  RtcPlayer *player = (RtcPlayer*)playerCtx;
  return player->Snapshot(_path.getString());
}

static jstring
com_youku_lflive_LFLiveAPI_GetLogStat(JNIEnv* env, jobject thiz, jlong playerCtx){
  return env->NewStringUTF("");
  //	LOGI("com_youku_lflive_LFLiveAPI_GetLogStat");
  //
  //	char tmpstr[512]={0};
  //
  //	const char* nullstr = "";
  //	LogStat* logstat = new LogStat();
  //	int ret = rtcGetLogStat((void *)playerCtx,logstat);
  //
  //	if (ret == 0 && logstat != NULL){
  //		const char* sid = (logstat->session_id == NULL) ? nullstr : logstat->session_id ;
  //		const char* alias = (logstat->alias == NULL) ? nullstr : logstat->alias ;
  //		const char* streamid = (logstat->streamid == NULL) ? nullstr : logstat->streamid ;
  //
  //		int video_frame_rate = logstat->video_frame_rate;
  //		int block_times = logstat->block_times;
  //		int replay_times = logstat->replay_times;
  //		int rttmp = sprintf(tmpstr, "{\"session_id\":\"%s\",\"alias\":\"%s\",\"stream_id\":\"%s\",\"video_frame_rate\":\"%d\",\"block_times\":\"%d\",\"replay_times\":\"%d\"}",
  //							sid, alias, streamid, video_frame_rate, block_times, replay_times);
  ////		LOGI("com_youku_lflive_LFLiveAPI_GetLogStat tmpstr:%s, rt%d",tmpstr, rttmp);
  //
  //		delete logstat;
  //		logstat = NULL;
  //	}
  //
  //	jstring rtstr = env->NewStringUTF(tmpstr);
  //
  //	return rtstr;
}

static JNINativeMethod mMethods[] = {//method for JAVA. use this to register native method
  { "native_setup", "()V", (void*)com_youku_lflive_LFLiveAPI_setup },
  { "native_register", "(Ljava/lang/Object;)V", (void*)com_youku_lflive_LFLiveAPI_register },
  { "native_unregister", "()V", (void*)com_youku_lflive_LFLiveAPI_unregister },
  { "native_CreatePlayer", "()J", (void*)com_youku_lflive_LFLiveAPI_CreatePlayer },
  { "native_StartPlay", "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)I", (void*)com_youku_lflive_LFLiveAPI_StartPlay },
  { "native_SetWindow", "(JLjava/lang/Object;)I", (void*)com_youku_lflive_LFLiveAPI_SetWindow },
  { "native_AdvancedInitPlayer", "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)I", (void*)com_youku_lflive_LFLiveAPI_AdvancedInitPlayer },
  { "native_SetDownloadParams", "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIIII)V", (void*)com_youku_lflive_LFLiveAPI_SetDownloadParams },
  { "native_SetNetworkChanged", "(J)I", (void*)com_youku_lflive_LFLiveAPI_SetNetworkChanged },
  { "native_getPlaylist", "(J)Ljava/lang/String;", (void*)com_youku_lflive_LFLiveAPI_native_getPlaylist },
  { "native_AdvancedStartPlay", "(J)I", (void*)com_youku_lflive_LFLiveAPI_AdvancedStartPlay },
  { "native_GetDownloadIp", "(J)Ljava/lang/String;", (void*)com_youku_lflive_LFLiveAPI_GetDownloadIp },
  { "native_GetDownloadUdpPort", "(J)I", (void*)com_youku_lflive_LFLiveAPI_GetDownloadUdpPort },
  { "native_GetDownloadTcpPort", "(J)I", (void*)com_youku_lflive_LFLiveAPI_GetDownloadTcpPort },
  { "native_GetDownloadHttpPort", "(J)I", (void*)com_youku_lflive_LFLiveAPI_GetDownloadHttpPort },
  { "native_GetDownloadStreamId", "(J)Ljava/lang/String;", (void*)com_youku_lflive_LFLiveAPI_GetDownloadStreamId },
  { "native_StopPlay", "(J)I", (void*)com_youku_lflive_LFLiveAPI_StopPlay },
  { "native_DestroyPlayer", "(J)I", (void*)com_youku_lflive_LFLiveAPI_DestroyPlayer },
  { "native_SetAVMute", "(JI)I", (void*)com_youku_lflive_LFLiveAPI_SetAVMute },
  { "native_SetAVMode", "(JI)I", (void*)com_youku_lflive_LFLiveAPI_SetAVMode },
  { "native_AdvancedSetDownloadReportPlayDelay", "(JZ)I", (void*)com_youku_lflive_LFLiveAPI_AdvancedSetDownloadReportPlayDelay },
  { "native_GetLogStat", "(J)Ljava/lang/String;", (void*)com_youku_lflive_LFLiveAPI_GetLogStat },
  { "native_SetOsVersion", "(Ljava/lang/String;)I", (void *)com_youku_lflive_LFLiveAPI_SetOsVersion },
  { "native_getFramesPerSecond", "(J)F", (void *)com_youku_lflive_LFLiveAPI_getFramesPerSecond },
  { "native_getBitRate", "(J)J", (void *)com_youku_lflive_LFLiveAPI_getBitRate },
  { "native_getAvgKeyFrameSize", "(J)J", (void *)com_youku_lflive_LFLiveAPI_getAvgKeyFrameSize },
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

int JNI_OnLoad(JavaVM* vm, void* reserved)
{//JNI main
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

