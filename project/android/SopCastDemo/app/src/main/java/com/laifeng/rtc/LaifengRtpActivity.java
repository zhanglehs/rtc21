package com.laifeng.rtc;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.graphics.Point;
import android.media.AudioManager;
import android.net.ConnectivityManager;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AlertDialog;
import android.text.TextUtils;
import android.util.Log;
import android.view.Display;
import android.view.LayoutInflater;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.Spinner;
import android.widget.Toast;

import com.laifeng.rtc.ui.MultiToggleImageButton;
import com.laifeng.rtpmediasdk.capturer.RtcCapturer;
import com.laifeng.rtpmediasdk.player.RtpPlayer;
import com.laifeng.rtpmediasdk.common.CommonUtils;

public class LaifengRtpActivity extends Activity {
    private static final String TAG = "LaifengRtp";

    private SurfaceView mLFLiveView;
    private MultiToggleImageButton mMicBtn;
    private MultiToggleImageButton mFlashBtn;
    private MultiToggleImageButton mFaceBtn;
    private MultiToggleImageButton mBeautyBtn;
    private MultiToggleImageButton mFocusBtn;
    private Button mRecordBtn;
    private boolean isBeauty;
    private Button mPlayBtn;
    private Button mLogToScreenBtn;
    private boolean isPlaying;
    private ProgressBar mProgressConnecting;
    private AlertDialog liveDialog;
    private Spinner mLiveHost;
    private EditText mLiveAppId;
    private EditText mLiveAlias;
    private EditText mLiveVideoBps;

    private Button mLiveNormal;

    private AlertDialog playDialog;
    private EditText mPlayAppId;
    private EditText mPlayAlias;
    private EditText mPlayAlias2;
    private Spinner mPlayHost;
    private Spinner mPlayLog;
    private EditText mPlayStreamId2;
    private Button mPlayNormal;
    private SurfaceView mSurfaceView;
    private SurfaceView mSurfaceView2;

    private RtcCapturer mCapturer;
    private RtpPlayer mLFPlayer;
    private RtpPlayer mLFPlayer2;
    private ListView mListView;
    private Handler handler = new Handler();
    private boolean mLogToScreen = true;

    private boolean mEnablePlayer2 = false;

    private EventBroadcast mEventBroadcast;
    private class EventBroadcast extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (ConnectivityManager.CONNECTIVITY_ACTION.equals(intent.getAction())) {
                int netstate = CommonUtils.getNetworkState(context);
                Log.d(TAG, "netstate : " + netstate);
                if(isPlaying)
                    mLFPlayer.SetNetworkChanged();
                if(netstate == CommonUtils.NETWORN_NONE)
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(LaifengRtpActivity.this, "请检查网络连接", Toast.LENGTH_SHORT).show();
                        }
                    });
            }else if (AudioManager.ACTION_HEADSET_PLUG.equals(intent.getAction())) {
                AudioManager localAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
                boolean isHeadsetOn = localAudioManager.isWiredHeadsetOn();
                AudioManager audioManager = (AudioManager)context.getSystemService(Context.AUDIO_SERVICE);
                if(isPlaying) {
                    if (isHeadsetOn) {
                        //audioManager.setMode(AudioManager.MODE_NORMAL);
                        audioManager.setSpeakerphoneOn(false);
                    } else {
                        //audioManager.setMode(AudioManager.MODE_IN_COMMUNICATION);
                        audioManager.setSpeakerphoneOn(true);
                    }
                }
            }
        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        RtpPlayer.register(this);
        setContentView(R.layout.activity_laifeng_rtp);
        initViews();
        initListeners();
        createDialogs();
        prepareRtpPlayer();

        mEventBroadcast = new EventBroadcast();
        IntentFilter filter = new IntentFilter();
        filter.addAction(android.net.ConnectivityManager.CONNECTIVITY_ACTION);
        filter.addAction(AudioManager.ACTION_HEADSET_PLUG);
        registerReceiver(mEventBroadcast, filter);
    }

    private void initViews() {
        mLFLiveView = (SurfaceView) findViewById(R.id.liveView);
        mMicBtn = (MultiToggleImageButton) findViewById(R.id.record_mic_button);
        mFlashBtn = (MultiToggleImageButton) findViewById(R.id.camera_flash_button);
        mFaceBtn = (MultiToggleImageButton) findViewById(R.id.camera_switch_button);
        mBeautyBtn = (MultiToggleImageButton) findViewById(R.id.camera_render_button);
        mFocusBtn = (MultiToggleImageButton) findViewById(R.id.camera_focus_button);
        mRecordBtn = (Button) findViewById(R.id.btnRecord);
        mPlayBtn = (Button) findViewById(R.id.btnPlay);
        mLogToScreenBtn = (Button) findViewById(R.id.btnLogToScreen);
        mProgressConnecting = (ProgressBar) findViewById(R.id.progressConnecting);
        mSurfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        mSurfaceView.setZOrderOnTop(false);
        mSurfaceView.setZOrderMediaOverlay(true);

        mSurfaceView2 = (SurfaceView) findViewById(R.id.surfaceView2);
        if (mEnablePlayer2) {
            mSurfaceView2.setZOrderOnTop(false);
            mSurfaceView2.setZOrderMediaOverlay(true);
        }
        mListView = (ListView) findViewById(R.id.debugInfoList);
        mListView.setStackFromBottom(true);
    }

    private void initListeners() {
        mMicBtn.setOnStateChangeListener(new MultiToggleImageButton.OnStateChangeListener() {
            @Override
            public void stateChanged(View view, int state) {
            }
        });
        mFlashBtn.setOnStateChangeListener(new MultiToggleImageButton.OnStateChangeListener() {
            @Override
            public void stateChanged(View view, int state) {
            }
        });
        mFaceBtn.setOnStateChangeListener(new MultiToggleImageButton.OnStateChangeListener() {
            @Override
            public void stateChanged(View view, int state) {
            }
        });
        mBeautyBtn.setOnStateChangeListener(new MultiToggleImageButton.OnStateChangeListener() {
            @Override
            public void stateChanged(View view, int state) {
                if(!isBeauty) {
                    isBeauty = true;
                } else {
                    isBeauty = false;
                }
            }
        });
        mFocusBtn.setOnStateChangeListener(new MultiToggleImageButton.OnStateChangeListener() {
            @Override
            public void stateChanged(View view, int state) {
            }
        });

        mRecordBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                int ret = -1;
                if (mCapturer == null) {
                    mCapturer = new RtcCapturer();
                    mCapturer.Create();
                    ret = mCapturer.StartCapture("", "", 640, 360);
                    ret = mCapturer.StartPreview(mLFLiveView.getHolder().getSurface());
                } else {
                    mCapturer.Stop();
                    mCapturer = null;
                }

                if (mCapturer == null) {
                    mRecordBtn.setText("开播");
                    mListView.setVisibility(View.GONE);
                } else {
                    mRecordBtn.setText("停止开播");
                    SharedPreferences sharedPreferences = getSharedPreferences("liveparams", Context.MODE_PRIVATE);
                    String livealias = sharedPreferences.getString("livealias", "");
                    if (!livealias.isEmpty()) {
                        mLiveAlias.setText(livealias);
                    }
                    liveDialog.show();
                }
            }
        });

        mPlayBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if(!isPlaying) {
                    showPlayDialog();
                } else {
                    stopRtpPlayer();
                }
            }
        });

        mLogToScreenBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mLogToScreen = !mLogToScreen;
            }
        });
    }

    private void createDialogs() {
        LayoutInflater inflater = getLayoutInflater();
        View playView = inflater.inflate(R.layout.rtp_play_dialog,(ViewGroup) findViewById(R.id.dialog));

        mPlayAppId = (EditText) playView.findViewById(R.id.appId);
        mPlayAlias = (EditText) playView.findViewById(R.id.alias);
        mPlayAlias2 = (EditText) playView.findViewById(R.id.alias2);
        mPlayHost = (Spinner) playView.findViewById(R.id.playhostspinner);
        mPlayLog = (Spinner) playView.findViewById(R.id.playlogspinner);
        mPlayStreamId2 = (EditText) playView.findViewById(R.id.downloadstreamid2);
        mPlayNormal = (Button) playView.findViewById(R.id.normalPlay);
        if (!mEnablePlayer2) {
            mPlayAlias2.setVisibility(View.INVISIBLE);
            mPlayStreamId2.setVisibility(View.INVISIBLE);
        }
        AlertDialog.Builder playBuilder = new AlertDialog.Builder(this);
        playBuilder.setTitle("播放设置");
        playBuilder.setView(playView);
        playDialog = playBuilder.create();

        mPlayNormal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
            playDialog.dismiss();
            String appId = mPlayAppId.getText().toString();
            String alias = mPlayAlias.getText().toString();
            Boolean alias2IsEmpty = false;
            if (mEnablePlayer2) {
                String alias2 = mPlayAlias2.getText().toString();
                if(TextUtils.isEmpty(alias2))
                    alias2IsEmpty = true;
            }
            String host = mPlayHost.getSelectedItem().toString();
            if(TextUtils.isEmpty(appId) || TextUtils.isEmpty(alias)|| alias2IsEmpty || TextUtils.isEmpty(host)) {
                Toast.makeText(LaifengRtpActivity.this, "请输入正确的播放参数", Toast.LENGTH_SHORT).show();
            } else {
                mProgressConnecting.setVisibility(View.VISIBLE);
                playRtpStream(false);
                SharedPreferences sharedPreferences = getSharedPreferences("playparams", Context.MODE_PRIVATE);
                SharedPreferences.Editor editor = sharedPreferences.edit();
                editor.putString("playalias", alias);
                editor.commit();
            }
            }
        });

        View liveView = inflater.inflate(R.layout.rtp_live_dialog,(ViewGroup) findViewById(R.id.dialog));

        mLiveHost = (Spinner) liveView.findViewById(R.id.livehostspinner);
        mLiveAppId = (EditText) liveView.findViewById(R.id.appId);
        mLiveAlias = (EditText) liveView.findViewById(R.id.alias);
        mLiveVideoBps = (EditText) liveView.findViewById(R.id.videobps);
        mLiveNormal = (Button) liveView.findViewById(R.id.normalLive);
        AlertDialog.Builder liveBuilder = new AlertDialog.Builder(this);
        liveBuilder.setTitle("直播设置");
        liveBuilder.setView(liveView);
        liveDialog = liveBuilder.create();

        mLiveNormal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
            String appId = mLiveAppId.getText().toString();
            String host = mLiveHost.getSelectedItem().toString();
            String alias = mLiveAlias.getText().toString();
            String videobps = mLiveVideoBps.getText().toString();
            if(TextUtils.isEmpty(appId) || TextUtils.isEmpty(alias) || TextUtils.isEmpty(host) || TextUtils.isEmpty(videobps)) {
                Toast.makeText(LaifengRtpActivity.this, "请输入正确的直播参数", Toast.LENGTH_SHORT).show();
            } else {
                if (mCapturer != null) {
                    mCapturer.StartSend(host, appId, alias, "98765", 640, 360, CommonUtils.getInt(videobps, 800) * 1024);
                }
                SharedPreferences sharedPreferences = getSharedPreferences("liveparams", Context.MODE_PRIVATE);
                SharedPreferences.Editor editor = sharedPreferences.edit();
                editor.putString("livealias", alias);
                editor.commit();
                liveDialog.dismiss();
            }
            }
        });
    }

    private synchronized void prepareRtpPlayer() {
        Log.d(TAG, "prepareRtpPlayer.");
        if (mLFPlayer != null) {
            return;
        }
        mLFPlayer = new RtpPlayer();
        mLFPlayer.setCallback(mRtpPlayerCallback);
        mLFPlayer.CreatePlayer();
        mLFPlayer.SetWindow(mSurfaceView.getHolder().getSurface());

        if (mEnablePlayer2) {
            if (mLFPlayer2 != null) {
                return;
            }
            mLFPlayer2 = new RtpPlayer();
            mLFPlayer2.setCallback(mRtpPlayerCallback);
            mLFPlayer2.CreatePlayer();
            mLFPlayer2.SetWindow(mSurfaceView2.getHolder().getSurface());
        }
    }

    private synchronized void playRtpStream(boolean isAdvanced) {
        Log.d(TAG, "playRtpStream.");
        String appId = mPlayAppId.getText().toString();
        String alias = mPlayAlias.getText().toString();
        String host = mPlayHost.getSelectedItem().toString();
        int logLevel = 0;
        if(mPlayLog.getSelectedItem().toString().equalsIgnoreCase("SEVER-LEVEL") == true)
            logLevel = 0;
        else if(mPlayLog.getSelectedItem().toString().equalsIgnoreCase("CLIENT-NON") == true)
            logLevel = 1;
        else if(mPlayLog.getSelectedItem().toString().equalsIgnoreCase("CLIENT-RTP") == true)
            logLevel = 2;
        mLFPlayer.StartPlay(appId, alias, host,"98765",CommonUtils.getUniquePsuedoID(),logLevel);
        if (mEnablePlayer2) {
            String alias2 = mPlayAlias2.getText().toString();
            mLFPlayer2.StartPlay(appId, alias2, host, "98765",CommonUtils.getUniquePsuedoID(),logLevel);
        }
    }

    private RtpPlayer.LFLiveCallback mRtpPlayerCallback = new RtpPlayer.LFLiveCallback() {
        @Override
        public void CallBackMessageFromNative(Object thiz,int msgid,String content, int wParam, int lParam) {
            final int err = msgid;
            RtpPlayer player = (RtpPlayer)thiz;
            if (player == null) {
                return;
            }
            boolean is_player1 = false;
            if (player == mLFPlayer) {
                is_player1 = true;
            }
            else if (player != mLFPlayer2) {
                return;
            }
            final boolean player1 = is_player1;

            if (msgid == RtpPlayer.LFLiveCallback.RTC_PLAYER_STATE_ERROR) {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(LaifengRtpActivity.this, "播放失败,错误码：" + err, Toast.LENGTH_SHORT).show();
                        mProgressConnecting.setVisibility(View.GONE);
                        stopRtpPlayer();
                    }
                });
            }
            else if (msgid == RtpPlayer.LFLiveCallback.RTC_PLAYER_MSG_VIDEO_RESOLUTION) {
                final float width = (float)wParam;
                final float height = (float)lParam;
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        SurfaceView playerView = mSurfaceView2;
                        if (player1) {
                            playerView = mSurfaceView;
                        }
                        RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams) playerView.getLayoutParams();
                        Display display = getWindowManager().getDefaultDisplay();
                        Point point = new Point();
                        display.getSize(point);
                        float large_view_width = point.x;
                        lp.width = (int) (large_view_width / 3.0f);
                        lp.height = (int) (lp.width * (height / width));
                        playerView.setLayoutParams(lp);
                    }
                });
            }
            else if (msgid == RtpPlayer.LFLiveCallback.RTC_PLAYER_STATE_INITIALIZING) {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        mProgressConnecting.setVisibility(View.VISIBLE);
                        mPlayBtn.setText("播放");
                    }
                });
            }
            else if (msgid == RtpPlayer.LFLiveCallback.RTC_PLAYER_MSG_VIDEO_FIRST_FRAME) {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        mProgressConnecting.setVisibility(View.GONE);
                        mPlayBtn.setText("停止");
                        isPlaying = true;

                        AudioManager localAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
                        boolean isHeadsetOn = localAudioManager.isWiredHeadsetOn();
                        AudioManager audioManager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
                        if (!isHeadsetOn) {
                            audioManager.setSpeakerphoneOn(true);
                        }

                        if (player1) {
                            mSurfaceView.setVisibility(View.VISIBLE);
                        } else {
                            mSurfaceView2.setVisibility(View.VISIBLE);
                        }
                    }
                });
            }
        }
    };

    private synchronized void stopRtpPlayer() {
        Log.d(TAG, "stopRtpPlayer.");
        isPlaying = false;
        if (mLFPlayer != null) {
            mLFPlayer.StopPlay();
        }
        if (mLFPlayer2 != null) {
            mLFPlayer2.StopPlay();
        }

        mSurfaceView.setVisibility(View.GONE);
        mSurfaceView2.setVisibility(View.GONE);
        mPlayBtn.setText("播放");
    }

    private void showPlayDialog() {
        mPlayAlias.setText(mLiveAlias.getText());
        SharedPreferences sharedPreferences = getSharedPreferences("playparams", Context.MODE_PRIVATE);
        String playalias = sharedPreferences.getString("playalias", "");
        if (!playalias.isEmpty())
            mPlayAlias.setText(playalias);
        if(mEnablePlayer2)
            mPlayAlias2.setText(mLiveAlias.getText());
        playDialog.show();
    }

    @Override
    public void onBackPressed(){
        destoryUploadAndDownload();
        super.onBackPressed();
    }

    @Override
    protected void onStop() {
        //isPlaying = false;
        super.onStop();
//        if(mRtpController != null)
//            mRtpController.pauseLive();
    }

    @Override
    protected void onStart() {
        super.onStart();
//        if(mRtpController != null)
//            mRtpController.resumeLive();
    }


    protected void destoryUploadAndDownload() {
        isPlaying = false;
        if (mLFPlayer != null) {
            Log.d(TAG, "StopPlay at onDestroy");
            mLFPlayer.StopPlay();
            mLFPlayer.DestroyPlayer();
            mLFPlayer = null;
        }

        RtpPlayer.unregister();
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy " + LaifengRtpActivity.this);
        destoryUploadAndDownload();
        unregisterReceiver(mEventBroadcast);
        super.onDestroy();
        if (mCapturer != null) {
            mCapturer.Stop();
            mCapturer = null;
        }
    }

}
