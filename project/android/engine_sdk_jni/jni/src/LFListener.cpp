#include "LFListener.h"

#define NULL 0

LFListener::LFListener(JNIEnv* env, jobject listener_object) {
  m_jvm = NULL;
  m_listener_object = NULL;
  m_listener_methodid = NULL;

  if (env == NULL) {
    return;
  }
  JavaVM *jvm = NULL;
  env->GetJavaVM(&jvm);
  if (jvm == NULL) {
    return;
  }
  jclass clazz = env->GetObjectClass(listener_object);
  if (clazz == NULL) {
    return;
  }

  m_jvm = jvm;
  m_listener_object = env->NewGlobalRef(listener_object);
  m_listener_methodid = env->GetMethodID(clazz, "CallbackMessageFromNative", "(ILjava/lang/String;II)V");
  env->DeleteLocalRef(clazz);
}

LFListener::~LFListener() {
  JNIEnv* env = NULL;
  if (m_jvm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
    return;
  }
  env->DeleteGlobalRef(m_listener_object);
  m_listener_object = NULL;
  m_listener_methodid = NULL;
}

void LFListener::notifyMessage(int msgid, long wParam, long lParam) {
  if (m_listener_methodid == NULL) {
    return;
  }

  JNIEnv* env = NULL;
  bool isAttached = false;
  if (m_jvm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
    jint res = m_jvm->AttachCurrentThread(&env, NULL);
    if ((res < 0) || !env) {
      return;
    }
    isAttached = true;
  }

  jstring jmsg = env->NewStringUTF("");
  env->CallVoidMethod(m_listener_object, m_listener_methodid, msgid, jmsg, (int)wParam, (int)lParam);

  if (isAttached) {
    m_jvm->DetachCurrentThread();
  }
}
