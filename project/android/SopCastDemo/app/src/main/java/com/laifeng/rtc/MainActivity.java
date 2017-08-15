package com.laifeng.rtc;

import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;

import com.laifeng.rtpmediasdk.player.RtpPlayer;

public class MainActivity extends AppCompatActivity {
    private long mLastRenderTime = 0;
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Context context = getApplicationContext();

        setContentView(R.layout.activity_main);
        GridView grid = (GridView) findViewById(R.id.grid);
        grid.setAdapter(new HoloTilesAdapter());

        try{
            System.loadLibrary("avengine_dll");
            System.loadLibrary("engine_sdk_jni");
            RtpPlayer.init(context);
        }catch(Exception ex)
        {
             String str = ex.getMessage();
             Log.d("MainActivity", "Couldn't load libs, " + str);
        }
    }

    public class HoloTilesAdapter extends BaseAdapter {

        private static final int TILES_COUNT = 6;

        private final int[] DRAWABLES = {
                R.drawable.blue_tile,
                R.drawable.green_tile,
                R.drawable.purple_tile,
                R.drawable.yellow_tile,
                R.drawable.red_tile,
                R.drawable.pink_tile
        };

        @Override
        public int getCount() {
            return TILES_COUNT;
        }

        @Override
        public Object getItem(int position) {
            return null;
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {

            RelativeLayout v;
            if (convertView == null) {
                v = (RelativeLayout) getLayoutInflater().inflate(R.layout.grid_item, parent, false);
            } else {
                v = (RelativeLayout) convertView;
            }
            v.setBackgroundResource(DRAWABLES[position % TILES_COUNT]);

            TextView textView1 = (TextView) v.findViewById(R.id.textView1);
            TextView textView2 = (TextView) v.findViewById(R.id.textView2);

            String string1 = "", string2 = "";
            if(position == 0) {
                string1 = "网络探测测试";
                string2 = "连接性";
            } else if(position == 1) {
                string1 = "上传下载测试";
                string2 = "华北";
            } else if(position == 2) {
                string1 = "上传下载测试";
                string2 = "华东";
            } else if(position == 3) {
                string1 = "上传下载测试";
                string2 = "华南";
            } else if(position == 4) {
                string1 = "Portrait";
                string2 = "房间聊天";
            } else if(position == 5) {
                string1 = "Portrait";
                string2 = "上下播";
            }
            textView1.setText(string1);
            textView2.setText(string2);

            final int currentPosition = position;
            v.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if(System.currentTimeMillis() - mLastRenderTime < 500)
                        return;
                    mLastRenderTime = System.currentTimeMillis();
                    if(currentPosition == 5) {
                        goLaifengRtp();
                    }
                }
            });
            return v;
        }
    }

    private void goLaifengRtp() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR2) {
            Toast.makeText(MainActivity.this, "系统版本过低，请使用Android 4.4以上的手机", Toast.LENGTH_SHORT).show();
            return;
        }
        Intent intent = new Intent(this, LaifengRtpActivity.class);
        startActivity(intent);
    }
}

