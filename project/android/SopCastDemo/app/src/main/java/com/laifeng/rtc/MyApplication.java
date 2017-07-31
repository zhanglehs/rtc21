package com.laifeng.rtc;

import android.app.Application;
import android.content.Context;

/**
 * Created by wangjz on 2017/4/5.
 */

public class MyApplication extends Application {
    private static Context context = null;
    @Override
    public void onCreate(){
        super.onCreate();
        context = getApplicationContext();
    }
    public static Context getAppContext(){
        return context;
    }
}
