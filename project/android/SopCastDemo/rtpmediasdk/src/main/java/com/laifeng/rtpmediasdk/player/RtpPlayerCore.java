package com.laifeng.rtpmediasdk.player;

import android.content.Context;
import android.util.Log;
import android.view.Surface;

import java.lang.ref.WeakReference;

/**
 * Created by elicyan on 17/4/28.
 */

public class RtpPlayerCore implements IPlayerCore {
    private static final String TAG = "Rtp-Video-Core";
    private RtpPlayer mCurrentPlayer;
    private IPlayerListener mPlayerListener;
    private Surface  mSurface;

    static {
        try{
            System.loadLibrary("webrtcdemo-jni");
            System.loadLibrary("engine_sdk_jni");
        } catch(Exception ex)
        {
            ex.printStackTrace();
        }
    }

    public RtpPlayerCore(Context context) {
        RtpPlayer.init(context);
        RtpPlayer.register(context);
    }


    @Override
    public boolean isPlaying() {
        if(mCurrentPlayer == null) {
            return false;
        }
        return mCurrentPlayer.isPlaying();
    }

    @Override
    public boolean isPause() {
        return false;
    }

    @Override
    public boolean isOperable() {
        return true;
    }

    @Override
    public boolean snapShot(String outPath) {
        if(mCurrentPlayer == null) {
            return false;
        }
        return mCurrentPlayer.snapShot(outPath);
    }

    @Override
    public int getVideoWidth() {
        if(mCurrentPlayer == null) {
            return -1;
        }
        return mCurrentPlayer.getVideoWidth();
    }

    @Override
    public int getVideoHeight() {
        if(mCurrentPlayer == null) {
            return -1;
        }
        return mCurrentPlayer.getVideoHeight();
    }

    public void init() {
        mCurrentPlayer = new RtpPlayer();
        mCurrentPlayer.setCallback(mRtpPlayerCallback);
        mCurrentPlayer.CreatePlayer();
    }

    @Override
    public void setUrl(String url) {
        if(mCurrentPlayer == null) {
            return;
        }
    }

    @Override
    public void setListener(IPlayerListener listener) {
        mPlayerListener = listener;
    }

    @Override
    public void setIpAndPort(String ip, int port) {
    }

    @Override
    public void reset() {
        if(mCurrentPlayer == null) {
            return;
        }
        mCurrentPlayer.StopPlay();
    }

    @Override
    public void release() {
        if(mCurrentPlayer == null) {
            return;
        }
        mCurrentPlayer.DestroyPlayer();
        mCurrentPlayer.unregister();
    }

    @Override
    public void setSurface(Surface surface) {
        if(mCurrentPlayer == null) {
            return;
        }
        mSurface = surface;
        mCurrentPlayer.SetWindow(mSurface);
    }

    @Override
    public void prepare() {
        if(mCurrentPlayer == null) {
            return;
        }
    }

    @Override
    public long getDuration() {
        if(mCurrentPlayer == null) {
            return 0;
        }
        return mCurrentPlayer.getDuration();
    }

    @Override
    public long getCurrentPosition() {
        if(mCurrentPlayer == null) {
            return 0;
        }
        return mCurrentPlayer.getCurrentPosition();
    }

    @Override
    public void seekTo(long position) {
        if(mCurrentPlayer == null) {
            return;
        }
        mCurrentPlayer.seekTo(position);
    }

    @Override
    public void pause() {
        if(mCurrentPlayer == null || !isPlaying()) {
            return;
        }
        mCurrentPlayer.pause();
    }

    @Override
    public void start() {
        if(mCurrentPlayer == null && mCurrentPlayer.isPlaying() == true) {
            return;
        }
    }

    public int startPlay(String appid, String alias,String host, String token,String userid,String extraparams /* "" */,int logLevel/* "0:server,1:non,2:dbg" */)
    {

        return mCurrentPlayer.StartPlay(appid,alias,host,token,userid,extraparams,logLevel);
    }

    @Override
    public long getPlayableDuration() {
        if(mCurrentPlayer == null) {
            return 0;
        }
        return 0;
    }
    @Override
    public void openLog(boolean isOpen) {
        if(mCurrentPlayer == null) {
            return;
        }
    }
    @Override
    public void mute(boolean mute) {
        if(mCurrentPlayer == null) {
            return;
        }
        mCurrentPlayer.SetAVMute(mute ? 1 : 0 );
    }

    @Override
    public void setLooping(boolean looping) {
        if(mCurrentPlayer == null) {
            return;
        }
    }

    @Override
    public float getFramesPerSecond() {
        if(mCurrentPlayer == null) {
            return 0;
        }
        return mCurrentPlayer.getFramesPerSecond();
    }

    @Override
    public long getBitRate() {
        if(mCurrentPlayer == null) {
            return 0;
        }
        return mCurrentPlayer.getBitRate();
    }

    @Override
    public long getAvgKeyFrameSize() {
        if(mCurrentPlayer == null) {
            return 0;
        }
        return mCurrentPlayer.getAvgKeyFrameSize();
    }

    public synchronized int SetNetworkChanged(){
        return mCurrentPlayer.SetNetworkChanged();
    }

    private RtpPlayer.LFLiveCallback mRtpPlayerCallback = new RtpPlayer.LFLiveCallback() {
        @Override
        public void CallBackMessageFromNative(Object weak_thiz,int msgid, String content, int wParam, int lParam) {
            final int err = msgid;
            if (msgid == RtpPlayer.LFLiveCallback.RTC_PLAYER_STATE_ERROR) {
                mPlayerListener.onPlayerError();
            }
//            else if (msgid == RtpPlayer.LFLiveCallback.RTC_PLAYER_VIDEO_RESOLUTION) {
//            }
//            else if (msgid == RtpPlayer.LFLiveCallback.RTC_PLAYER_INIT) {
//            }
//            else if (msgid == RtpPlayer.LFLiveCallback.RTC_PLAYER_FIRST_VIDEO_FRAME) {
//            }
//            else if (msgid == RtpPlayer.LFLiveCallback.RTC_PLAYER_FIRST_AUDIO_FRAME) {
//            }
//            else if (msgid == RtpPlayer.LFLiveCallback.RTC_PLAYER_BUFFERING_START) {
//                if(mPlayerListener != null) {
//                    mPlayerListener.onPlayerLoading();
//                }
//            }
//            else if (msgid == RtpPlayer.LFLiveCallback.RTC_PLAYER_BUFFERING_END) {
//                if(mPlayerListener != null) {
//                    mPlayerListener.onPlayerEndLoading();
//                }
//            }
        }
    };


}
