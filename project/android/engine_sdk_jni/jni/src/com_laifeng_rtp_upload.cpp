#include "engine_api/rtp_upload.h"
#include "engine_api/RtcLog.h"
#include "LFListener.h"
#include "engine_api/rtp_api.h"

#include <vector>
#include <string>
#include <stdlib.h>

//#define LFRTP_LOG_PATH "/sdcard/lfrtplog"
#ifdef TAG
#undef TAG
#endif
#define TAG "LFUpload"

static const char* UploadClassName = "com/laifeng/rtpmediasdk/capturer/RtcCapturer";

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

static void
CapturerMessageCallback(RtcCapture* ctx, unsigned int msgid, int wParam, int lParam) {
  LFListener *listener = (LFListener *)ctx->GetUserdata();
  if (listener != NULL){
    listener->notifyMessage(msgid, wParam, lParam);
  }
}

static jlong
jni_Create(JNIEnv* env, jobject thiz) {
  INF("%s", __FUNCTION__);
  RtcCapture *capturer = new RtcCapture("xxxdeviceidxxx");
  capturer->SetUserdata(new LFListener(env, thiz));
  return (jlong)capturer;
}

static void
jni_Destroy(JNIEnv* env, jobject thiz, jlong jniCtx) {
  INF("%s", __FUNCTION__);
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
  INF("%s", __FUNCTION__);
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
  INF("%s", __FUNCTION__);
  if (jniCtx == 0) {
    return -1;
  }

  RtcCapture *capturer = (RtcCapture*)jniCtx;
  return capturer->StartPreview(surface);
}

static jint
jni_StartSend(JNIEnv* env, jobject thiz, jlong jniCtx, jstring jlapi, jstring jappid, jstring jalias,
              jstring jtoken, jint video_encode_width, jint video_encode_height, jint video_bitrate) {
  INF("%s", __FUNCTION__);
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

  // INFO: zhangle, create stream is test code
  char streamid[256] = { 0 };
  char url[1024];
  sprintf(url, "http://%s/v1/create_stream?app_id=%s&alias=%s&stream_type=rtp&res=%dx%d&rt=400&stream_format=rtp&nt=1&token=98765&p2p=0",
          lapi.getString(), appid.getString(), alias.getString(), video_encode_width, video_encode_height);
  INF("%s, create stream, url=%s", __FUNCTION__, url);
  create_stream_sync(streamid, url);
  INF("%s, create stream finished", __FUNCTION__);

  return capturer->StartEncodeAndSend(&net, &encode, CapturerMessageCallback);
}

static void
jni_StopSend(JNIEnv* env, jobject thiz, jlong jniCtx) {
  INF("%s", __FUNCTION__);
  if (jniCtx == 0) {
    return;
  }

  RtcCapture *capturer = (RtcCapture*)jniCtx;
  capturer->StopEncodeAndSend();
}

static void
jni_StopPreview(JNIEnv* env, jobject thiz, jlong jniCtx) {
  INF("%s", __FUNCTION__);
  if (jniCtx == 0) {
    return;
  }

  RtcCapture *capturer = (RtcCapture*)jniCtx;
  capturer->StopPreview();
}

static void
jni_Stop(JNIEnv* env, jobject thiz, jlong jniCtx) {
  INF("%s", __FUNCTION__);
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
    ERR("Native registration unable to find class '%s'\n", classname);
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
