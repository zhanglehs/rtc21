package com.laifeng.rtpmediasdk.common;

import android.util.Log;

public class LiveRtpLog {
    public static final String TAG = "LiveRtp";

    private static boolean open = false;

    public static void isOpen(boolean isOpen) {
        open = isOpen;
    }

    public static void d(String tag, String msg) {
        if(open) {
            Log.d(tag, msg);
        }
    }

    public static void w(String tag, String msg) {
        if(open) {
            Log.w(tag, msg);
        }
    }

    public static void e(String tag, String msg) {
        if(open) {
            Log.e(tag, msg);
        }
    }

    public static void d(String msg) {
        if(open) {
            Log.d(TAG, msg);
        }
    }

    public static void w(String msg) {
        if(open) {
            Log.w(TAG, msg);
        }
    }

    public static void e(String msg) {
        if(open) {
            Log.e(TAG, msg);
        }
    }
}
