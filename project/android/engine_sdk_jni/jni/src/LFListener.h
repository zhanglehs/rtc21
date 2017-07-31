#ifndef LISTENER_H_
#define LISTENER_H_

#include <jni.h>

// 该类用于jni中事件通知java的相应类，
// 每次调用notifyMessage，会导致java对象listener_object的
// void CallbackMessageFromNative(int msgid, String content, int wParam, int lParam)被回调。
//
// 构造函数与析构函数必须在java线程上调用
class LFListener {
public:
	LFListener(JNIEnv* env, jobject listener_object);
	~LFListener();
	void notifyMessage(int msgid, long wParam, long lParam);

private:
	JavaVM* m_jvm;
	jobject m_listener_object;
	jmethodID m_listener_methodid;
};

#endif /* LISTENER_H_ */
