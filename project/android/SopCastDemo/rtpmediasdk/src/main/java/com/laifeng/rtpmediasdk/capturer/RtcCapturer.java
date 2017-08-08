package com.laifeng.rtpmediasdk.capturer;

import android.util.Log;
import android.view.Surface;

public class RtcCapturer {

	private static final String TAG = RtcCapturer.class.getSimpleName();

	private native long native_Create();
	private native void native_Destroy(long ctx);
	private native int native_StartCapture(long ctx, String audio_deviceid, String video_deviceid,
										   int video_capture_width, int video_capture_height);
	private native int native_StartPreview(long ctx, Object surface);
	private native int native_StartSend(long ctx, String lapi, String appid, String alias, String token, int video_encode_width, int video_encode_height, int video_bitrate);
	private native void native_StopSend(long ctx);
	private native void native_StopPreview(long ctx);
	private native void native_Stop(long ctx);

	private long jniCtx = 0;

	public void Create() {
		jniCtx = native_Create();
	}

	public void Destroy() {
		native_Destroy(jniCtx);
		jniCtx = 0;
	}

	public int StartCapture(String audio_deviceid, String video_deviceid,
							int video_capture_width, int video_capture_height)  {
		return native_StartCapture(jniCtx, audio_deviceid, video_deviceid, video_capture_width, video_capture_height);
	}

	public int StartPreview(Surface surface) {
		return native_StartPreview(jniCtx, surface);
	}

	public int StartSend(String lapi, String appid, String alias, String token, int video_encode_width, int video_encode_height, int video_bitrate) {
		return native_StartSend(jniCtx, lapi, appid, alias, token, video_encode_width, video_encode_height, video_bitrate);
	}

	public void StopSend() {
		native_StopSend(jniCtx);
	}

	public void StopPreview() {
		native_StopPreview(jniCtx);
	}

	public void Stop() {
		native_Stop(jniCtx);
	}

	public static interface LFLiveCallback{
		// 这些常量请与jni层的RtcPlayerNotifyMessageId保持一致
		public static final int RTC_PLAYER_STATE_BASE = 100;
		public static final int RTC_PLAYER_STATE_UNSET = RTC_PLAYER_STATE_BASE;
		public static final int RTC_PLAYER_STATE_INITIALIZING = RTC_PLAYER_STATE_BASE + 1;
		public static final int RTC_PLAYER_STATE_RUNNING = RTC_PLAYER_STATE_BASE + 2;
		public static final int RTC_PLAYER_STATE_ERROR = RTC_PLAYER_STATE_BASE + 3;
		public static final int RTC_PLAYER_STATE_STOPPED = RTC_PLAYER_STATE_BASE + 4;

		public static final int RTC_PLAYER_MSG_BASE = RTC_PLAYER_STATE_STOPPED + 100;
		public static final int RTC_PLAYER_MSG_VIDEO_RESOLUTION = RTC_PLAYER_MSG_BASE;
		public static final int RTC_PLAYER_MSG_VIDEO_FIRST_FRAME = RTC_PLAYER_MSG_BASE + 1;
		public static final int RTC_PLAYER_MSG_VIDEO_SNAPSHOT = RTC_PLAYER_MSG_BASE + 2;
		public void CallBackMessageFromNative(Object weak_thiz, int msgid, String content, int wParam, int lParam);
	}

	private LFLiveCallback mCallback;

	public void setCallback(LFLiveCallback cb) {
		mCallback = cb;
	}

	private void CallbackMessageFromNative(int msgid, String content, int wParam, int lParam) {
		Log.i(TAG, "CallbackMessageFromNative msgid:" + msgid + ", content: " + content);

		if (mCallback != null) {
			mCallback.CallBackMessageFromNative(this, msgid, content, wParam, lParam);
		}
	}
}
