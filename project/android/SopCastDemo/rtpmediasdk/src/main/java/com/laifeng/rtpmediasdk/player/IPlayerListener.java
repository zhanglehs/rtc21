package com.laifeng.rtpmediasdk.player;

/**
 * Created by elicyan on 17/4/28.
 */

public interface IPlayerListener {
    void onPlayerStart();

    void onPlayerLoading();

    void onPlayerEndLoading();

    void onPlayerError();

    void onPlayerComplete();

    void onRotationChange(int var1);
}