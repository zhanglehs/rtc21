package com.laifeng.rtpmediasdk.player;

import android.view.Surface;

/**
 * Created by elicyan on 17/4/28.
 */

public interface IPlayerCore {
    boolean isPlaying();

    boolean isPause();

    boolean isOperable();

    boolean snapShot(String var1);

    int getVideoWidth();

    int getVideoHeight();

    void init();

    void setUrl(String var1);

    void setListener(IPlayerListener var1);

    void setIpAndPort(String var1, int var2);

    void reset();

    void release();

    void setSurface(Surface var1);

    void prepare();

    void openLog(boolean var1);

    long getDuration();

    long getCurrentPosition();

    void seekTo(long var1);

    void pause();

    void start();

    long getPlayableDuration();

    void mute(boolean var1);

    void setLooping(boolean var1);

    float getFramesPerSecond();

    long getBitRate();

    long getAvgKeyFrameSize();
}