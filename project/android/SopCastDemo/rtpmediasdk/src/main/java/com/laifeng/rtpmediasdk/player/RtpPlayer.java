package com.laifeng.rtpmediasdk.player;

import android.content.Context;
import android.os.Build;
import android.os.PowerManager;
import android.util.Log;
import android.view.Surface;
import java.lang.ref.WeakReference;

public class RtpPlayer {

	private static final String TAG = RtpPlayer.class.getSimpleName();

	private native static void native_register(Object context);
	private native static void native_unregister();
	private native void native_setup();

	private native long native_CreatePlayer();
	private native int native_StartPlay(long ctx,
										String appid, String alias, String host,String token,String deviceid,String extraparams,int logLevel);
	private native int native_SetWindow(long ctx, Object surface);
	private native int native_AdvancedInitPlayer(long ctx,String appid, String alias,String host,
												 String token,String deviceid,int logLevel);
	private native void native_SetDownloadParams(long ctx,String ip,String udpport,String tcpport,
												 String httpport,String streamid,int enablefec,int enablenack,
												 int is_lostpacketStrategy,int IlostpacketToScreen,int PlostpacketToScreen);
	private native int native_AdvancedStartPlay(long ctx);
	private native int native_AdvancedSetDownloadReportPlayDelay(long ctx,boolean enable);
	private native int native_StopPlay(long ctx);
	private native int native_SetNetworkChanged(long ctx);
	private native String native_getPlaylist(long ctx);
	private native int native_SetAVMute(long ctx,int status);
	private native int native_SetAVMode(long ctx,int avmode);
	private native int native_DestroyPlayer(long ctx);
	private native String native_GetLogStat(long ctx);
	private native String native_GetDownloadIp(long ctx);
	private native int native_GetDownloadUdpPort(long ctx);
	private native int native_GetDownloadTcpPort(long ctx);
	private native int native_GetDownloadHttpPort(long ctx);
	private native String native_GetDownloadStreamId(long ctx);
	private native static int native_SetOsVersion(String osVersion);
	public native float native_getFramesPerSecond(long ctx);//帧率
	public native long native_getBitRate(long ctx);//码率
	public native long native_getAvgKeyFrameSize(long ctx);//I帧平均大小
	public native boolean native_snapShot(long ctx,String var1);

	private static boolean mRegistered = false;
	private static PowerManager.WakeLock mWakeLock = null;
	public LFLiveCallback mCallback;
	private long jniPlayerCtx = 0;
	private int mWidth;
	private int mHight;
	private boolean mIsPlaying;

	public RtpPlayer() {
		native_setup();
	}

	public void setCallback(LFLiveCallback cb)
	{
		mCallback = cb;
	}

	public static void register(Context context) {
		synchronized (RtpPlayer.class) {
			Log.d(TAG, "register");
			if (mRegistered == false) {
				mRegistered = true;
				if (mWakeLock != null) {
					if (mWakeLock.isHeld()) {
						mWakeLock.release();
					}
					mWakeLock = null;
				}

				PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
				mWakeLock = pm.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK |PowerManager.ON_AFTER_RELEASE, TAG);
				mWakeLock.setReferenceCounted(false);
				mWakeLock.acquire();
			}
		}
	}

	public static void unregister(){
		synchronized (RtpPlayer.class) {
			Log.d(TAG, "unregister");
			if (mRegistered == true) {
				mRegistered = false;

				if (mWakeLock != null) {
					if (mWakeLock.isHeld()) {
						mWakeLock.release();
					}
					mWakeLock = null;
				}
			}
		}
	}

	public static void init(Context context) {
		native_register(context.getApplicationContext());
	}

	public static void uninit() {
		native_unregister();
	}

	public int CreatePlayer(){
		jniPlayerCtx = native_CreatePlayer();
		return 0;
	}

	public int DestroyPlayer(){
		Log.e(TAG, "DestroyPlayer");
		native_DestroyPlayer(jniPlayerCtx);
		jniPlayerCtx = 0;
		mIsPlaying = false;
		return 0;
	}

	public int StartPlay(String appid,
						 String alias,String host, String token,String deviceid,String extraparams,int logLevel){
		Log.d(TAG, "StartPlay "+"appid:"+appid+",alias:"+alias+",host:"+host+ ",token:"+token);
		int ret = native_StartPlay(jniPlayerCtx, appid, alias, host,token,deviceid,extraparams,logLevel);
		if(ret == 0)
			mIsPlaying = true;
		return ret;
	}

	public int SetWindow(Surface surface) {
		return native_SetWindow(jniPlayerCtx, surface);
	}

	public synchronized int AdvancedInitPlayer(String appid, String alias,String host, String token,String deviceid,int logLevel){
		Log.d(TAG, "AdvancedInitPlay "+"appid:"+appid+",alias:"+alias+",host:"+host+ ",token:"+token);
		return native_AdvancedInitPlayer(jniPlayerCtx,appid, alias, host,token, deviceid,logLevel);
	}
	public synchronized void SetDownloadParams(String ip,String udpport,String tcpport,String httpport,
								  String streamid,int enablefec,int enablenack,int is_lostpacketStrategy,
								  int IlostpacketToScreen,int PlostpacketToScreen){
		native_SetDownloadParams(jniPlayerCtx,ip, udpport, tcpport,httpport,streamid,enablefec,
				enablenack,is_lostpacketStrategy,IlostpacketToScreen,PlostpacketToScreen);
	}
	public synchronized int AdvancedStartPlay(){
		Log.d(TAG, "AdvancedStartPlay");
		return native_AdvancedStartPlay(jniPlayerCtx);
	}
	public synchronized int AdvancedSetDownloadReportPlayDelay(boolean enalbe){
		Log.d(TAG, "AdvancedSetDownloadReportPlayDelay");
		return native_AdvancedSetDownloadReportPlayDelay(jniPlayerCtx,enalbe);
	}

	public synchronized String GetDownloadIp(){
		return native_GetDownloadIp(jniPlayerCtx);
	}
	public synchronized int GetDownloadUdpPort(){
		return native_GetDownloadUdpPort(jniPlayerCtx);
	}
	public synchronized int GetDownloadTcpPort(){
		return native_GetDownloadTcpPort(jniPlayerCtx);
	}
	public synchronized int GetDownloadHttpPort(){
		return native_GetDownloadHttpPort(jniPlayerCtx);
	}
	public synchronized String GetDownloadStreamId(){
		return native_GetDownloadStreamId(jniPlayerCtx);
	}
	public synchronized int StopPlay(){
		Log.e(TAG, "StopPlay");
		mIsPlaying = false;
        return native_StopPlay(jniPlayerCtx);
	}

	public synchronized boolean isPlaying(){
		return mIsPlaying;
	}
	public synchronized int SetAVMute(int states){
		Log.d(TAG, "SetAVMute " + "states:"+states);
		return native_SetAVMute(jniPlayerCtx,states);
	}

	public synchronized int SetAVMode(int avmode){
		Log.d(TAG, "SetAVMode " + "avmode:"+avmode);
		return native_SetAVMode(jniPlayerCtx,avmode);
	}

	public synchronized float getFramesPerSecond()//帧率
	{
		return native_getFramesPerSecond(jniPlayerCtx);
	}

	public synchronized long getBitRate()//码率
	{
		return native_getBitRate(jniPlayerCtx);
	}
	public synchronized long getAvgKeyFrameSize()//I帧平均大小
	{
		return native_getAvgKeyFrameSize(jniPlayerCtx);
	}
	public synchronized boolean snapShot(String var1)
	{
		return native_snapShot(jniPlayerCtx,var1);
	}

	public synchronized int getVideoWidth() {return mWidth;}

	public synchronized int getVideoHeight() {return mHight;}

	public synchronized long getDuration() {return 0;}

	public synchronized long getCurrentPosition() {return 0;}

	public synchronized void seekTo(long var1) {}

	public synchronized void pause(){}

	public synchronized String GetLogStat(){
		Log.d(TAG, "GetLogStat");
		return native_GetLogStat(jniPlayerCtx);
	}

	private void CallbackMessageFromNative(int msgid, String content, int wParam, int lParam) {
		Log.i(TAG, "CallBackMessageFromNative msgid:" + msgid + ", content: " + content);

        if (msgid == RtpPlayer.LFLiveCallback.RTC_PLAYER_MSG_VIDEO_RESOLUTION) {
            mWidth = wParam;
            mHight = lParam;
        }

		if (mCallback != null) {
			mCallback.CallBackMessageFromNative(this, msgid, content, wParam, lParam);
		}
	}


	public synchronized int SetNetworkChanged(){
		Log.d(TAG, "SetNetworkChanged");
		return native_SetNetworkChanged(jniPlayerCtx);
	}

	public synchronized String getPlaylist(){
		Log.d(TAG, "getPlaylist");
		return native_getPlaylist(jniPlayerCtx);
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

	static {
		 String osVersion = "android " + Build.VERSION.RELEASE;
		 native_SetOsVersion(osVersion);
	}
}
