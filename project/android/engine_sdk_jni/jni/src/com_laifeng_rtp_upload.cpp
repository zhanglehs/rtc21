#include "engine_api/rtp_upload.h"
#include "engine_api/RtcLog.h"
#include "myLog.h"
#include "engine_api/RtcCapture.h"
#include "LFListener.h"

#include <jni.h>
#include <android/log.h>
#include <assert.h>
#include <string.h>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <stdlib.h>

//#define LFRTP_LOG_PATH "/sdcard/lfrtplog"
#ifdef TAG
#undef TAG
#endif
#define TAG "LFUpload"

static const char* UploadClassName = "com/laifeng/rtpmediasdk/capturer/RtcCapturer";
//static const char* SendReportClassName = "com/laifeng/rtpmediasdk/uploader/SendReport";
//static const char* VideoEstimateClassName = "com/laifeng/rtpmediasdk/uploader/VideoEstimate";
//
//
//static JavaVM* gjVM = NULL;
//
//using namespace live_stream_sdk;
//using namespace std;
//
//RTPUpload* rtp_upload = NULL;
////SessionDescription * _sdp = NULL;
//jclass clazz = NULL;
//jmethodID methodMessageCB = NULL;
////LogLevel log_level = LOG_LEVEL_RTP;
//
//#define ROTATE_LEFT(x, s, n) ((x) << (n)) | ((x) >> ((s) - (n)))
//#define ROTATE_RIGHT(x, s, n) ((x) >> (n)) | ((x) << ((s) - (n)))
//#define MAX_BUFFER_SZIE 1920 * 1080 * 3 /2
//
////static jbyteArray ret_data;
//static char dst[MAX_BUFFER_SZIE];
//
//typedef enum MediaFormate{
//  FLV = 1,
//  RTP,
//  RTMP,
//}MediaFormate;

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

/*static void lfs_log_callback(int level, const char *file, int line_num, const char *format, va_list args)
{
  //time_t t;
  //struct tm now_time;
  //char buf[1024 * 8];

  //if (NULL == format || level < log_level)
  //    return;

  //t = time(NULL);
  //if (localtime_r(&t, &now_time) == NULL) {
  //    return;
  //}

  ////const char *log_type = log_level2str(level);

  //int ret = snprintf(buf, sizeof(buf) -1, "%02d:%02d:%02d %d [%s:%d]",
  //    now_time.tm_hour, now_time.tm_min, now_time.tm_sec,
  //     level, file, line_num);

  //if (ret > 0)
  //{
  //    ret += vsnprintf(buf + ret, sizeof(buf)-ret - 1, format, args);
  //}

  //LOGI("%s", buf);
}

void log_to_logcat(int level, const char *file, int line_num, const char *format, va_list args)
{
  //char buf[1024 * 16];
  //buf[0] = 0;
  //live_stream_sdk::get_log_str(buf, sizeof(buf), level, file, line_num, format, args);

  //switch (level )
  //{
  //    case live_stream_sdk::LOG_LEVEL_TRC:
  //    {
  //        LOGV("%s",buf);
  //        break;
  //    }
  //    case live_stream_sdk::LOG_LEVEL_DBG:
  //    {
  //        LOGD("%s",buf);
  //        break;
  //    }
  //    case live_stream_sdk::LOG_LEVEL_INF:
  //    {
  //        LOGI("%s",buf);
  //        break;
  //    }
  //    case live_stream_sdk::LOG_LEVEL_WRN:
  //    {
  //        LOGW("%s",buf);
  //        break;
  //    }
  //    case live_stream_sdk::LOG_LEVEL_ERR:
  //    {
  //        LOGE("%s",buf);
  //        break;
  //    }
  //}
}


void init_lfs_log(int logLevel)
{
  //mkdir(LFRTP_LOG_PATH,0777);
  //  if(logLevel == 1)
  //   LogSetLevel(LOG_LEVEL_NON);
  //  else if(logLevel == 2)
  //   LogSetLevel(LOG_LEVEL_DBG);
  //LogSetCallback(log_to_logcat,LFRTP_LOG_PATH,live_stream_sdk::PlayerType::UPLOADER);
}

class AudioSpecificConfig
{
public:
  uint8_t a : 5;
  uint8_t b : 4;
  uint8_t c : 4;
  uint8_t d : 1;
  uint8_t e : 1;
  uint8_t f : 1;
};


static void uploader_message_callback(unsigned int msgid, const char* content) {
  JNIEnv* env = NULL;
  bool isAttached = false;
  if (gjVM->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
    jint res = gjVM->AttachCurrentThread(&env, NULL);
    if ((res < 0) || !env) {
      return;
    }
    isAttached = true;
  }

  if (methodMessageCB)
  {
    jstring jmsg = env->NewStringUTF(content);
    env->CallStaticVoidMethod(clazz, methodMessageCB, (int)msgid, jmsg);
  }

  if (isAttached) {
    gjVM->DetachCurrentThread();
  }
}

int
com_laifeng_livestreamsdk_UploadStart(JNIEnv* env, jobject thiz, jstring deviceid, jstring appid, jstring alias, jstring host, jstring token, jstring extraparams, jint logLevel)
{
  return 0;
  //	init_lfs_log(logLevel);
  //	int ret = -1;
  //    LOGI("com_laifeng_livestreamsdk_UploadStart +++");
  //    CJstringToString _deviceid(env, deviceid);
  //
  //    if (rtp_upload != NULL){
  //        INF("com_laifeng_livestreamsdk_UploadStop %p +++", rtp_upload);
  //        LOGI("com_laifeng_livestreamsdk_UploadStop: %p",rtp_upload);
  //        rtp_upload->Stop();
  //        delete rtp_upload;
  //        rtp_upload = NULL;
  //    }
  //
  //    if(_sdp != NULL)
  //    {
  //        delete _sdp;
  //        _sdp= NULL;
  //    }
  //
  //	rtp_upload = new RTPUpload(_deviceid.getString());
  //    if(rtp_upload == NULL) {
  //        LOGI("com_laifeng_livestreamsdk_UploadStart ---");
  //        return ret;
  //    }
  //
  //    jclass clz = env->FindClass(UploadClassName);
  //    clazz = (jclass)env->NewGlobalRef(clz);
  //    if (clazz == NULL) {
  //        return -1;
  //    }
  //
  //    methodMessageCB = env->GetStaticMethodID(clazz, "CallBackMessageFromNative", "(ILjava/lang/String;)V");
  //    if(methodMessageCB == NULL)
  //        return -1;
  //
  //    CJstringToString _stream_alias(env,alias);
  //    CJstringToString _appid(env,appid);
  //    CJstringToString _host(env,host);
  //    CJstringToString _token(env,token);
  //    CJstringToString _extraparams(env,extraparams);
  ////    if (_host.compare("101.201.57.242") == 0) {
  ////            std::string repurl = _host+":6601";
  ////            LiveStreamSDKSetReportUrl(repurl.c_str(),"/v1/upload_file");
  ////        }
  //    rtp_upload->SetLapiUrl(_host.getString());
  //    rtp_upload->RegisterOnStateCallback(&uploader_message_callback);
  //
  //    LOGI("laifeng-debug UploadStart:%p", rtp_upload);
  //	ret = rtp_upload->Init(_appid.getString(),_stream_alias.getString(),_token.getString(),_extraparams.getString());
  //    LOGI("com_laifeng_livestreamsdk_UploadStart %p ret:%d ---", rtp_upload,ret);
  //	return ret;
}

int
com_laifeng_livestreamsdk_SetNetworkChanged(JNIEnv* env, jobject thiz)
{
  if (rtp_upload == NULL)
    return -1;
  return rtp_upload->SetNetworkChanged();
}

int com_laifeng_livestreamsdk_EnableFec(JNIEnv* env, jobject thiz, jint flag)
{
  if (rtp_upload == NULL)
  {
    return -1;
  }
  rtp_upload->EnableFec((bool)flag);
  return 0;
}

int com_laifeng_livestreamsdk_EnableNack(JNIEnv* env, jobject thiz, jint flag)
{
  if (rtp_upload == NULL)
  {
    return -1;
  }
  rtp_upload->EnableNack((bool)flag);
  return 0;
}

void com_laifeng_livestreamsdk_SetUploadParams(JNIEnv* env, jobject thiz, jstring uploadip, jstring uploadudpport, jstring uploadtcpport, jstring uploadhttpport, jstring uploadstreamid)
{
  //if(rtp_upload == NULL)
  //    return;
  //CJstringToString _uploadip(env,uploadip);
  //CJstringToString _uploadudpport(env,uploadudpport);
  //CJstringToString _uploadtcpport(env,uploadtcpport);
  //CJstringToString _uploadhttpport(env,uploadhttpport);
  //CJstringToString _uploadstreamid(env,uploadstreamid);
  //rtp_upload->SetReceiverIP(_uploadip.getString());
  //rtp_upload->SetReceiverUDPPort(atoi(_uploadudpport.getString()));
  //rtp_upload->SetReceiverTCPPort(atoi(_uploadtcpport.getString()));
  //rtp_upload->SetUploadToken("98765");
  //rtp_upload->SetStreamId(_uploadstreamid.getString());
  //char sdpurl[1024];
  //sprintf(sdpurl,"http://%s:%s/upload/sdp/%s.sdp?token=98765",
  //        _uploadip.getString(),_uploadhttpport.getString(),_uploadstreamid.getString());
  //LOGI("upload sdp url:%s",sdpurl);
  //rtp_upload->SetReceiverSDPUrl(sdpurl);
}

jstring com_laifeng_livestreamsdk_GetUploadIp(JNIEnv* env, jobject thiz)
{
  return env->NewStringUTF("");
  //if(rtp_upload == NULL)
  //    return env->NewStringUTF("");
  //const char *ipaddr = rtp_upload->GetReceiverIP();
  //if(ipaddr == NULL)
  //    return env->NewStringUTF("");
  //jstring ip = env->NewStringUTF(ipaddr);
  //LOGI("get upload ip:%s",ipaddr);
  //return ip;
}

int com_laifeng_livestreamsdk_SetOsVersion(JNIEnv* env, jobject thiz, jstring osVersion)
{
  //const char *c_osVersion = env->GetStringUTFChars(osVersion,NULL);
  //LogSetOsVersion(c_osVersion);
  //env->ReleaseStringUTFChars(osVersion,c_osVersion);
  return 0;
}

int com_laifeng_livestreamsdk_GetUploadUdpPort(JNIEnv* env, jobject thiz)
{
  return 0;
  //if(rtp_upload == NULL)
  //    return 0;
  //int udpport = rtp_upload->GetReceiverUDPPort();
  //LOGI("get upload udp port:%d",udpport);
  //return udpport;
}

int com_laifeng_livestreamsdk_GetUploadTcpPort(JNIEnv* env, jobject thiz)
{
  return 0;
  //if(rtp_upload == NULL)
  //    return 0;
  //int tcpport = rtp_upload->GetReceiverTCPPort();
  //LOGI("get upload tcp port:%d",tcpport);
  //return tcpport;
}

jstring com_laifeng_livestreamsdk_GetStreamId(JNIEnv* env, jobject thiz)
{
  return env->NewStringUTF("");
  //if(rtp_upload == NULL)
  //    return env->NewStringUTF("");
  //const char * pid = rtp_upload->GetStreamId();
  //if(pid == NULL)
  //    return env->NewStringUTF("");
  //jstring streamid = env->NewStringUTF(pid);
  //LOGI("get upload streamid:%s",pid);
  //return streamid;
}

int com_laifeng_livestreamsdk_GetUploadHttpPort(JNIEnv* env, jobject thiz)
{
  return 0;
  //if(rtp_upload == NULL)
  //    return 0;
  //const char * sdpurl = rtp_upload->GetReceiverSDPUrl();
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
}

jobject com_laifeng_livestreamsdk_GetSendReport(JNIEnv* env, jobject thiz, jobject srobj)
{
  //jclass objectClass = env->FindClass(SendReportClassName);
  ////获取类中每一个变量的定义 
  //jfieldID frac_lost_packet_count = env->GetFieldID(objectClass,"frac_lost_packet_count","I");
  //jfieldID total_lost_packet_count = env->GetFieldID(objectClass,"total_lost_packet_count","I"); 
  //jfieldID rtt_ms = env->GetFieldID(objectClass,"rtt_ms","I");
  //jfieldID total_bitrate = env->GetFieldID(objectClass,"total_bitrate","I");
  //jfieldID effect_bitrate = env->GetFieldID(objectClass,"effect_bitrate","I");
  //jfieldID packet_lost_rate = env->GetFieldID(objectClass,"packet_lost_rate","F");
  //if(rtp_upload == NULL)
  //    return srobj;
  //const SenderReport * psendRep = rtp_upload->GetSenderReport();

  //env->SetIntField(srobj,frac_lost_packet_count, psendRep->frac_lost_packet_count);
  //env->SetIntField(srobj,total_lost_packet_count, psendRep->total_lost_packet_count);
  //env->SetIntField(srobj,rtt_ms, psendRep->rtt_ms);
  //env->SetIntField(srobj,total_bitrate, psendRep->total_bitrate);
  //env->SetIntField(srobj,effect_bitrate, psendRep->effect_bitrate);
  //env->SetFloatField(srobj,packet_lost_rate, psendRep->packet_lost_rate);

  return srobj;
}

void com_laifeng_livestreamsdk_SetVideoExpectBitrate(JNIEnv* env, jobject thiz, jint bitrate)
{
  if (rtp_upload != NULL){
    rtp_upload->SetVideoExpectBitrate(bitrate);
  }
}

jobject com_laifeng_livestreamsdk_GetVideoEstimate(JNIEnv* env, jobject thiz, jobject srobj)
{
  jclass objectClass = env->FindClass(VideoEstimateClassName);
  //获取类中每一个变量的定义 
  jfieldID target_bitrate = env->GetFieldID(objectClass, "target_bitrate", "I");
  jfieldID lossrate = env->GetFieldID(objectClass, "lossrate", "F");
  jfieldID rtt_ms = env->GetFieldID(objectClass, "rtt", "I");

  uint32_t bitrate;
  uint8_t loss;
  int64_t rtt;
  if (rtp_upload == NULL)
    return srobj;
  rtp_upload->GetVideoEstimate(&bitrate, &loss, &rtt);
  env->SetIntField(srobj, target_bitrate, bitrate);
  env->SetFloatField(srobj, lossrate, (float)loss / 256.0);
  env->SetIntField(srobj, rtt_ms, (uint32_t)rtt);

  return srobj;
}

int
com_laifeng_livestreamsdk_UploadStop(JNIEnv* env, jobject thiz)
{
  //   INF("com_laifeng_livestreamsdk_UploadStop %p +++", rtp_upload);
  //   LOGI("com_laifeng_livestreamsdk_UploadStop:%p",rtp_upload);
  //if (rtp_upload != NULL){
  //	rtp_upload->Stop();
  //	delete rtp_upload;
  //	rtp_upload = NULL;
  //}

  //if(_sdp != NULL)
  //{
  //	delete _sdp;
  //	_sdp= NULL;
  //}
  //   if(clazz) {
  //       env->DeleteGlobalRef(clazz);
  //       clazz = NULL;
  //   }
  //   methodMessageCB = NULL;
  //   INF("com_laifeng_livestreamsdk_UploadStop %p ---", rtp_upload);
  return 0;

}


int
com_laifeng_livestreamsdk_UploadSend(JNIEnv* env, jobject thiz, jbyteArray  jdata, jint size)
{
  //char * data = (char *)(env)->GetByteArrayElements(jdata, 0);
  //int ret = -1;
  //if (rtp_upload != NULL){
  //	ret = rtp_upload->SendRTP((uint8_t *)(data), size);
  //}
  //(env)->ReleaseByteArrayElements(jdata,(jbyte*)data,0);

  //return ret;
  return 0;
}

void
com_laifeng_livestreamsdk_SetTimeout(JNIEnv* env, jobject thiz, jint startTimeout, jint reconnectTimeout)
{
  //if (rtp_upload != NULL){
  //    rtp_upload->SetOnTimeOutSecond(startTimeout, reconnectTimeout);
  //}
}

int
com_laifeng_livestreamsdk_SetSessionDescription(JNIEnv* env, jobject thiz, jobject jobj)
{
  return 0;
  //   INF("com_laifeng_livestreamsdk_SetSessionDescription %p +++", rtp_upload);
  //   LOGI("com_laifeng_livestreamsdk_SetSessionDescription");
  //   jclass sdp = (env)->GetObjectClass(jobj);

  //   if(sdp != NULL) {
  //       jfieldID aac_media_ssrc_ID =  (env)->GetFieldID(sdp,"aac_media_ssrc","I");
  //       int audio_ssrc =  (env)->GetIntField(jobj,aac_media_ssrc_ID);
  //       LOGI("set aac_media_ssrc %d", audio_ssrc);

  //       jfieldID h264_media_ssrc_ID =  (env)->GetFieldID(sdp,"h264_media_ssrc","I");
  //       int video_ssrc =  (env)->GetIntField(jobj,h264_media_ssrc_ID);
  //       LOGI("set h264_media_ssrc %d", video_ssrc);

  //   	if (rtp_upload != NULL){
  //           int ret =  rtp_upload->SetSessionDescription(audio_ssrc, video_ssrc);
  //           INF("com_laifeng_livestreamsdk_SetSessionDescription %p ret:%d ---", rtp_upload,ret);
  //           return ret;
  //   	}
  //   }
  //   INF("com_laifeng_livestreamsdk_SetSessionDescription %p ret: -1 ---", rtp_upload);
  //return -1;
}*/

static void
CapturerMessageCallback(RtcCapture* ctx, unsigned int msgid, int wParam, int lParam) {
  LFListener *listener = (LFListener *)ctx->GetUserdata();
  if (listener != NULL){
    listener->notifyMessage(msgid, wParam, lParam);
  }
}

static jlong
jni_Create(JNIEnv* env, jobject thiz) {
  LOGI("%s:%s", __FILE__, __FUNCTION__);

  RtcCapture *capturer = new RtcCapture("xxxdeviceidxxx");
  capturer->SetUserdata(new LFListener(env, thiz));
  return (jlong)capturer;
}

static void
jni_Destroy(JNIEnv* env, jobject thiz, jlong jniCtx) {
  LOGI("%s:%s", __FILE__, __FUNCTION__);
  if (jniCtx == 0) {
    return;
  }

  RtcCapture *capturer = (RtcCapture*)jniCtx;
  LFListener *listener = (LFListener*)capturer->GetUserdata();
  delete capturer;
  delete listener;
}

static jint
jni_StartCapture(JNIEnv* env, jobject thiz, jlong jniCtx, jstring jaudio_deviceid, jstring jvideo_deviceid,
                 jint video_capture_width, jint video_capture_height) {
  LOGI("%s:%s", __FILE__, __FUNCTION__);
  if (jniCtx == 0) {
    return -1;
  }

  CJstringToString audio_deviceid(env, jaudio_deviceid);
  CJstringToString video_deviceid(env, jvideo_deviceid);
  lfrtcCaptureConfig config;
  strcpy(config.audio_deviceid, audio_deviceid.getString());
  strcpy(config.video_deviceid, video_deviceid.getString());
  config.video_capture_width = video_capture_width;
  config.video_capture_height = video_capture_height;
  RtcCapture *capturer = (RtcCapture*)jniCtx;
  return capturer->StartCapture(&config);
}

static jint
jni_StartPreview(JNIEnv* env, jobject thiz, jlong jniCtx, jobject surface) {
  LOGI("%s:%s", __FILE__, __FUNCTION__);
  if (jniCtx == 0) {
    return -1;
  }

  RtcCapture *capturer = (RtcCapture*)jniCtx;
  return capturer->StartPreview(surface);
}

static jint
jni_StartSend(JNIEnv* env, jobject thiz, jlong jniCtx, jstring jlapi, jstring jappid, jstring jalias,
              jstring jtoken, jint video_encode_width, jint video_encode_height, jint video_bitrate) {
  LOGI("%s:%s", __FILE__, __FUNCTION__);
  if (jniCtx == 0) {
    return -1;
  }

  CJstringToString lapi(env, jlapi);
  CJstringToString appid(env, jappid);
  CJstringToString alias(env, jalias);
  CJstringToString token(env, jtoken);
  RtcCapture::NetworkConfig net;
  strcpy(net.lapi, lapi.getString());
  strcpy(net.appid, appid.getString());
  strcpy(net.alias, alias.getString());
  strcpy(net.token, token.getString());
  lfrtcEncodeConfig encode;
  encode.video_encode_width = video_encode_width;
  encode.video_encode_height = video_encode_height;
  encode.video_bitrate = video_bitrate;
  RtcCapture *capturer = (RtcCapture*)jniCtx;
  return capturer->StartEncodeAndSend(&net, &encode, CapturerMessageCallback);
}

static void
jni_StopSend(JNIEnv* env, jobject thiz, jlong jniCtx) {
  LOGI("%s:%s", __FILE__, __FUNCTION__);
  if (jniCtx == 0) {
    return;
  }

  RtcCapture *capturer = (RtcCapture*)jniCtx;
  capturer->StopEncodeAndSend();
}

static void
jni_StopPreview(JNIEnv* env, jobject thiz, jlong jniCtx) {
  LOGI("%s:%s", __FILE__, __FUNCTION__);
  if (jniCtx == 0) {
    return;
  }

  RtcCapture *capturer = (RtcCapture*)jniCtx;
  capturer->StopPreview();
}

static void
jni_Stop(JNIEnv* env, jobject thiz, jlong jniCtx) {
  LOGI("%s:%s", __FILE__, __FUNCTION__);
  if (jniCtx == 0) {
    return;
  }

  RtcCapture *capturer = (RtcCapture*)jniCtx;
  capturer->Stop();
}

static JNINativeMethod UploadMethods[] = {//method for JAVA. use this to register native method
  { "native_Create", "()J", (void*)jni_Create },
  { "native_Destroy", "(J)V", (void*)jni_Destroy },
  { "native_StartCapture", "(JLjava/lang/String;Ljava/lang/String;II)I", (void*)jni_StartCapture },
  { "native_StartPreview", "(JLjava/lang/Object;)I", (void*)jni_StartPreview },
  { "native_StartSend", "(JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;III)I", (void*)jni_StartSend },
  { "native_StopSend", "(J)V", (void*)jni_StopSend },
  { "native_StopPreview", "(J)V", (void*)jni_StopPreview },
  { "native_Stop", "(J)V", (void*)jni_Stop },
};

int jniRegisterClassNativeMethods(JNIEnv* env, const char * classname, const JNINativeMethod* methods, int count) {
  jclass clazz;
  int ret = -1;
  clazz = env->FindClass(classname);
  if (clazz == NULL) {
    LOGE("Native registration unable to find class '%s'\n", classname);
    return ret;
  }
  if ((ret = env->RegisterNatives(clazz, methods, count)) < 0) {
    //LOGE("RegisterNatives failed for '%s'\n", methods);
    return ret;
  }

  return ret;
}

int registerRtpUploadMethods(JNIEnv* env) {
  return jniRegisterClassNativeMethods(env, UploadClassName, UploadMethods, sizeof(UploadMethods) / sizeof(UploadMethods[0]));
}
