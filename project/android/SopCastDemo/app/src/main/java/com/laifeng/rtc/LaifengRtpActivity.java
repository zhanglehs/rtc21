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
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

//import com.laifeng.rtpmediasdk.uploader.LiveRtpController;
//import com.laifeng.rtpmediasdk.uploader.SendReport;
import com.laifeng.rtc.ui.MultiToggleImageButton;
//import com.laifeng.rtpmediasdk.uploader.UploadParams;
//import com.youku.laifeng.capture.camera.CameraListener;
//import com.youku.laifeng.capture.configuration.AudioConfiguration;
//import com.youku.laifeng.capture.configuration.VideoConfiguration;
//import com.youku.laifeng.capture.effect.video.color.BeautyEffect;
//import com.youku.laifeng.capture.effect.video.color.NullEffect;
import com.laifeng.rtpmediasdk.capturer.RtcCapturer;
import com.laifeng.rtpmediasdk.player.RtpPlayer;
import com.laifeng.rtpmediasdk.common.CommonUtils;
//import com.youku.laifeng.capture.effect.video.color.ColorEffect;
//import com.youku.laifeng.capture.effect.video.color.ShortVideoEffect;
//import com.youku.laifeng.capture.view.RenderCameraView;
//import com.youku.laifeng.livebase.data.StreamInfo;
//import com.youku.laifeng.livebase.listener.OnLiveListener;
//import com.youku.laifeng.livebase.listener.StartError;
//import com.youku.laifeng.livebase.listener.StopCase;

import java.io.IOException;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;

public class LaifengRtpActivity extends Activity {
    private static final String TAG = "LaifengRtp";

    private static final String CREATE_STREAM_URL = "http://%1s/v1/create_stream?" +
            "v=55&alias=%2s&stream_format=rtp&stream_type=rtp" +
            "&token=98765&nt=0&app_id=%3s&cl=xingmeng&rt=300&res=672x378";

    private static final String DESTROY_STREAM_URL = "http://%1s/v1/destroy_stream?" +
            "alias=%2s&token=98765&app_id=%3s";
    private SurfaceView mLFLiveView;
//    private LiveRtpController mRtpController;
    private MultiToggleImageButton mMicBtn;
    private MultiToggleImageButton mFlashBtn;
    private MultiToggleImageButton mFaceBtn;
    private MultiToggleImageButton mBeautyBtn;
    private MultiToggleImageButton mFocusBtn;
    private Button mRecordBtn;
    private boolean isBeauty;
    private boolean isRecording;
    private Button mPlayBtn;
    private Button mLogToScreenBtn;
    private boolean isPlaying;
    private ProgressBar mProgressConnecting;
    private AlertDialog liveDialog;
    private Spinner mLiveHost;
    private EditText mLiveAppId;
    private EditText mLiveAlias;
    private EditText mLiveVideoBps;
    private EditText mLiveEnableFec;
    private EditText mLiveEnableNack;
    private Spinner mLiveLog;
    private EditText mLiveUploadIP;
    private EditText mLiveUploadUdpPort;
    private EditText mLiveUploadTcpPort;
    private EditText mLiveUploadHttpPort;
    private EditText mLiveMtuSize;
    private EditText mLiveStreamId;

    private Button mLiveNormal;
    private LinearLayout mLiveMore;
    private Button mLiveDebug;
    private Button mCreateStream;
    private Switch mLiveSwitch;

    private AlertDialog playDialog;
    private Button mPlayDebug;
    private Button mGetStream;
    private Switch mPlaySwitch;
    private EditText mPlayAppId;
    private EditText mPlayAlias;
    private EditText mPlayAlias2;
    private Spinner mPlayHost;
    private Spinner mPlayLog;
    private EditText mPlayDownloadIP;
    private EditText mPlayDownloadUdpPort;
    private EditText mPlayDownloadTcpPort;
    private EditText mPlayDownloadHttpPort;
    private EditText mPlayIsLostPacketStrategy;
    private EditText mPlayILostPacketToScreen;
    private EditText mPlayPLostPacketToScreen;
    private EditText mPlayEnableFec;
    private EditText mPlayEnableNack;
    private EditText mPlayStreamId;
    private EditText mPlayStreamId2;
    private Button mPlayNormal;
    private LinearLayout mPlayMore;
    private SurfaceView mSurfaceView;
    private SurfaceView mSurfaceView2;

    private RtcCapturer mCapturer;
    private RtpPlayer mLFPlayer;
    private RtpPlayer mLFPlayer2;
//    private MyAdapter myAdapter;
    private ListView mListView;
    private Handler handler = new Handler();
    private int TIME = 1000;
    private Handler debughandler = new Handler();
//    private List<SendReport> sendReportList = new ArrayList<>();
    private int mBitrateCheckInterval = 0;
    private boolean mLogToScreen = true;

//    private ColorEffect mBeautyEffect;
//    private ColorEffect mNullEffect;
    String s1="MULTIPLAYER";
    String s2="MULTI"+"PLAYER";
    String str1="MULTI";
    String str2="PLAYER";
    //final String str1="MULTI";
    //final String str2="PLAYER";

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
        initLiveView();
        prepareRtpPlayer();

        mEventBroadcast = new EventBroadcast();
        IntentFilter filter = new IntentFilter();
        filter.addAction(android.net.ConnectivityManager.CONNECTIVITY_ACTION);
        filter.addAction(AudioManager.ACTION_HEADSET_PLUG);
        registerReceiver(mEventBroadcast, filter);
        debughandler.postDelayed(debugrunnable, TIME);
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

        if(s1 == str1 + str2) {
            mSurfaceView2 = (SurfaceView) findViewById(R.id.surfaceView2);
            mSurfaceView2.setZOrderOnTop(false);
            mSurfaceView2.setZOrderMediaOverlay(true);
        }
        mListView = (ListView) findViewById(R.id.debugInfoList);
//        myAdapter = new MyAdapter();
//        mListView.setAdapter(myAdapter);
        mListView.setStackFromBottom(true);
    }

    private void initListeners() {
        mMicBtn.setOnStateChangeListener(new MultiToggleImageButton.OnStateChangeListener() {
            @Override
            public void stateChanged(View view, int state) {
//                mRtpController.mute(true);
            }
        });
        mFlashBtn.setOnStateChangeListener(new MultiToggleImageButton.OnStateChangeListener() {
            @Override
            public void stateChanged(View view, int state) {
//                mRtpController.switchTorch();
            }
        });
        mFaceBtn.setOnStateChangeListener(new MultiToggleImageButton.OnStateChangeListener() {
            @Override
            public void stateChanged(View view, int state) {
//                mRtpController.switchCamera();
            }
        });
        mBeautyBtn.setOnStateChangeListener(new MultiToggleImageButton.OnStateChangeListener() {
            @Override
            public void stateChanged(View view, int state) {
                if(!isBeauty) {
//                    mRtpController.setColorEffect(mBeautyEffect);
                    isBeauty = true;
                } else {
//                    mRtpController.setColorEffect(mNullEffect);
                    isBeauty = false;
                }
            }
        });
        mFocusBtn.setOnStateChangeListener(new MultiToggleImageButton.OnStateChangeListener() {
            @Override
            public void stateChanged(View view, int state) {
//                mRtpController.switchFocusMode();
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
                    ret = mCapturer.StartSend("101.201.57.242", "301", "zhangle", "98765", 640, 360, 800000);

                } else {
                    mCapturer.Stop();
                    mCapturer = null;
                }
                if(isRecording) {
                    isRecording = false;
                    handler.post(new Runnable() {
                        @Override
                        public void run() {
                            mRecordBtn.setText("开播");
                            Toast.makeText(LaifengRtpActivity.this, "停止开播", Toast.LENGTH_SHORT).show();
                        }
                    });
                    destroyStream();
//                    if(sendReportList.size() > 0)
//                    {
//                        sendReportList.clear();
//                        myAdapter.notifyDataSetChanged();
//                    }
                    mListView.setVisibility(View.GONE);
//                    mRtpController.stopLive();
                } else {
                    SharedPreferences sharedPreferences = getSharedPreferences("liveparams", Context.MODE_PRIVATE);
                    String livealias = sharedPreferences.getString("livealias", "");
                    if(!livealias.isEmpty())
                        mLiveAlias.setText(livealias);
                    showLiveDialog();
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

//    private OnLiveListener mLiveListener = new OnLiveListener() {
//        @Override
//        public void onConnecting() {
//            handler.post(new Runnable() {
//                @Override
//                public void run() {
//                    mProgressConnecting.setVisibility(View.VISIBLE);
//                }
//            });
//        }
//
//        @Override
//        public void onLiving() {
//                handler.post(new Runnable() {
//                    @Override
//                    public void run() {
//                        Toast.makeText(LaifengRtpActivity.this, "开始直播", Toast.LENGTH_SHORT).show();
//                        mProgressConnecting.setVisibility(View.GONE);
//                        mRecordBtn.setText("停止");
//                    }
//                });
//            isRecording = true;
//        }
//
//        @Override
//        public void onReconnecting() {
//            handler.post(new Runnable() {
//                @Override
//                public void run() {
//                    mProgressConnecting.setVisibility(View.VISIBLE);
//                }
//            });
//        }
//
//        @Override
//        public void onReLiving() {
//            handler.post(new Runnable() {
//                @Override
//                public void run() {
//                    mProgressConnecting.setVisibility(View.GONE);
//
//                }
//            });
//        }
//
//        @Override
//        public void onStartError(StartError error) {
////            mRtpController.stopLive();
//            isRecording = false;
//            handler.post(new Runnable() {
//                 @Override
//                 public void run() {
//                     mProgressConnecting.setVisibility(View.GONE);
//                     Toast.makeText(LaifengRtpActivity.this, "直播失败", Toast.LENGTH_SHORT).show();
//                     mRecordBtn.setText("开播");
//                 }
//             });
//        }
//
//        @Override
//        public void onStop(StopCase stopCase) {
////            mRtpController.stopLive();
//            isRecording = false;
//            handler.post(new Runnable() {
//                 @Override
//                 public void run() {
//                     mProgressConnecting.setVisibility(View.GONE);
//                     Toast.makeText(LaifengRtpActivity.this, "直播停止", Toast.LENGTH_SHORT).show();
//                     mRecordBtn.setText("开播");
//
//                 }
//             });
//        }
//    };

    private void initLiveView() {
//        mNullEffect = new NullEffect(this);
//        mBeautyEffect = new ShortVideoEffect(this);
//        mRtpController = new LiveRtpController();
//        mRtpController.init(this);
//        mRtpController.setLiveListener(mLiveListener);
//        mRtpController.setRenderCameraView(mLFLiveView);
//        //设置预览监听
//        mRtpController.setCameraOpenListener(new CameraListener() {
//            @Override
//            public void onOpenSuccess() {
//                Toast.makeText(LaifengRtpActivity.this, "摄像头开启成功", Toast.LENGTH_LONG).show();
//            }
//
//            @Override
//            public void onOpenFail(int error) {
//                Toast.makeText(LaifengRtpActivity.this, "摄像头开启失败，错误码:" + error, Toast.LENGTH_LONG).show();
//            }
//
//            @Override
//            public void onCameraChange() {
//                Toast.makeText(LaifengRtpActivity.this, "摄像头切换", Toast.LENGTH_LONG).show();
//            }
//        });
//
//        mRtpController.setColorEffect(mBeautyEffect);
    }

    private void createDialogs() {
        LayoutInflater inflater = getLayoutInflater();
        View playView = inflater.inflate(R.layout.rtp_play_dialog,(ViewGroup) findViewById(R.id.dialog));

        mPlayAppId = (EditText) playView.findViewById(R.id.appId);
        mPlayAlias = (EditText) playView.findViewById(R.id.alias);
        mPlayAlias2 = (EditText) playView.findViewById(R.id.alias2);
        mPlayHost = (Spinner) playView.findViewById(R.id.playhostspinner);
        mPlayLog = (Spinner) playView.findViewById(R.id.playlogspinner);
        mPlayMore = (LinearLayout) playView.findViewById(R.id.moreSettings);
        mPlaySwitch = (Switch) playView.findViewById(R.id.switchSetting);
        mPlayDownloadIP = (EditText) playView.findViewById(R.id.downloadIp);
        mPlayDownloadUdpPort = (EditText) playView.findViewById(R.id.downloadUdpPort);
        mPlayDownloadTcpPort = (EditText) playView.findViewById(R.id.downloadTcpPort);
        mPlayDownloadHttpPort = (EditText) playView.findViewById(R.id.downloadHttpPort);
        mPlayIsLostPacketStrategy = (EditText) playView.findViewById(R.id.islostpacketstrategy);
        mPlayILostPacketToScreen = (EditText) playView.findViewById(R.id.ilostpackettoscreen);
        mPlayPLostPacketToScreen = (EditText) playView.findViewById(R.id.plostpackettoscreen);
        mPlayEnableFec = (EditText) playView.findViewById(R.id.enablefec);
        mPlayEnableNack = (EditText) playView.findViewById(R.id.enablenack);
        mPlayStreamId = (EditText) playView.findViewById(R.id.downloadstreamid);
        mPlayStreamId2 = (EditText) playView.findViewById(R.id.downloadstreamid2);
        mPlayNormal = (Button) playView.findViewById(R.id.normalPlay);
        mPlayDebug = (Button) playView.findViewById(R.id.debugPlay);
        mGetStream = (Button) playView.findViewById(R.id.getstream);
        if(s1 != str1 + str2) {
            mPlayAlias2.setVisibility(View.INVISIBLE);
            mPlayStreamId2.setVisibility(View.INVISIBLE);
        }
        AlertDialog.Builder playBuilder = new AlertDialog.Builder(this);
        playBuilder.setTitle("播放设置");
        playBuilder.setView(playView);
        playDialog = playBuilder.create();

        mPlaySwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if(isChecked) {
                    mPlayMore.setVisibility(View.VISIBLE);
                    mPlayNormal.setVisibility(View.GONE);
                } else {
                    mPlayMore.setVisibility(View.GONE);
                    mPlayNormal.setVisibility(View.VISIBLE);
                }
            }
        });

        mPlayNormal.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                playDialog.dismiss();
                String appId = mPlayAppId.getText().toString();
                String alias = mPlayAlias.getText().toString();
                Boolean alias2IsEmpty = false;
                if(s1 == str1 + str2) {
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
        mPlayDebug.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String downloadip = mPlayDownloadIP.getText().toString();
                String downloadudpport = mPlayDownloadUdpPort.getText().toString();
                String downloadtcpport = mPlayDownloadTcpPort.getText().toString();
                String downloadhttpport = mPlayDownloadHttpPort.getText().toString();
                String islostpacketstrategy = mPlayIsLostPacketStrategy.getText().toString();
                String ilostpackettoscreen = mPlayILostPacketToScreen.getText().toString();
                String plostpackettoscreen = mPlayPLostPacketToScreen.getText().toString();
                String enablefec = mPlayEnableFec.getText().toString();
                String enablenack = mPlayEnableNack.getText().toString();
                String downloadstreamid = mPlayStreamId.getText().toString();
                String downloadstreamid2 = downloadstreamid;
                Boolean downloadstreamid2IsEmpty = false;
                if(s1 == str1 + str2) {
                    downloadstreamid2 = mPlayStreamId2.getText().toString();
                    if(TextUtils.isEmpty(downloadstreamid2))
                        downloadstreamid2IsEmpty = true;
                }
                if(TextUtils.isEmpty(downloadip) ||TextUtils.isEmpty(downloadudpport) || TextUtils.isEmpty(downloadtcpport)
                    || TextUtils.isEmpty(downloadhttpport) || TextUtils.isEmpty(downloadstreamid) || downloadstreamid2IsEmpty
                        || TextUtils.isEmpty(enablefec) || TextUtils.isEmpty(enablenack) || TextUtils.isEmpty(islostpacketstrategy)
                        || TextUtils.isEmpty(ilostpackettoscreen)|| TextUtils.isEmpty(plostpackettoscreen)) {
                    Toast.makeText(LaifengRtpActivity.this, "请输入正确的播放参数", Toast.LENGTH_SHORT).show();
                } else {
                    mProgressConnecting.setVisibility(View.VISIBLE);
                    mLFPlayer.SetDownloadParams(downloadip,downloadudpport,downloadtcpport,downloadhttpport,downloadstreamid,CommonUtils.getInt(enablefec,0),CommonUtils.getInt(enablenack,1),CommonUtils.getInt(islostpacketstrategy,1),CommonUtils.getInt(ilostpackettoscreen,0),CommonUtils.getInt(plostpackettoscreen,0));
                    if(s1 == str1 + str2)
                        mLFPlayer2.SetDownloadParams(downloadip,downloadudpport,downloadtcpport,downloadhttpport,downloadstreamid2,CommonUtils.getInt(enablefec,0),CommonUtils.getInt(enablenack,1),CommonUtils.getInt(islostpacketstrategy,1),CommonUtils.getInt(ilostpackettoscreen,0),CommonUtils.getInt(plostpackettoscreen,0));
                    playRtpStream(true);
                    playDialog.dismiss();
                }
            }
        });

        mGetStream.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String appId = mPlayAppId.getText().toString();
                String alias = mPlayAlias.getText().toString();
                String alias2 = alias;
                Boolean alias2IsEmpty = false;
                if(s1 == str1 + str2) {
                    alias2 = mPlayAlias2.getText().toString();
                    if(TextUtils.isEmpty(alias2))
                        alias2IsEmpty = true;
                }
                String host = mPlayHost.getSelectedItem().toString();
                if(TextUtils.isEmpty(appId) || TextUtils.isEmpty(alias)|| alias2IsEmpty || TextUtils.isEmpty(host)) {
                    Toast.makeText(LaifengRtpActivity.this, "请输入正确的播放参数", Toast.LENGTH_SHORT).show();
                } else {
                    SharedPreferences sharedPreferences = getSharedPreferences("playparams", Context.MODE_PRIVATE);
                    SharedPreferences.Editor editor = sharedPreferences.edit();
                    editor.putString("playalias", alias);
                    editor.commit();
                    int logLevel = 0;
                    if(mPlayLog.getSelectedItem().toString().equalsIgnoreCase("SEVER-LEVEL") == true)
                        logLevel = 0;
                    else if(mPlayLog.getSelectedItem().toString().equalsIgnoreCase("CLIENT-NON") == true)
                        logLevel = 1;
                    else if(mPlayLog.getSelectedItem().toString().equalsIgnoreCase("CLIENT-RTP") == true)
                        logLevel = 2;
                    mLFPlayer.AdvancedInitPlayer(appId, alias, host,"98765",CommonUtils.getUniquePsuedoID(),logLevel);
                    if(s1 == str1 + str2)
                        mLFPlayer2.AdvancedInitPlayer(appId, alias2, host,"98765",CommonUtils.getUniquePsuedoID(),logLevel);
                    String downloadip = mLFPlayer.GetDownloadIp();
                    String downloadudpport = String.valueOf(mLFPlayer.GetDownloadUdpPort());
                    String downloadtcpport = String.valueOf(mLFPlayer.GetDownloadTcpPort());
                    String downloadhttpport = String.valueOf(mLFPlayer.GetDownloadHttpPort());
                    String downloadstreamid = mLFPlayer.GetDownloadStreamId();
                    Boolean streamid2IsEmpty = false;
                    if(s1 == str1 + str2) {
                        String downloadstreamid2 = mLFPlayer2.GetDownloadStreamId();
                        mPlayStreamId2.setText(downloadstreamid2);
                        if(TextUtils.isEmpty(downloadstreamid2))
                            streamid2IsEmpty = true;
                    }
                    mPlayDownloadIP.setText(downloadip);
                    mPlayDownloadUdpPort.setText(downloadudpport);
                    mPlayDownloadTcpPort.setText(downloadtcpport);
                    mPlayDownloadHttpPort.setText(downloadhttpport);
                    mPlayStreamId.setText(downloadstreamid);
                    if(TextUtils.isEmpty(downloadip) ||TextUtils.isEmpty(downloadudpport) || TextUtils.isEmpty(downloadtcpport)
                            || TextUtils.isEmpty(downloadhttpport)|| TextUtils.isEmpty(downloadstreamid)|| streamid2IsEmpty) {
                        Toast.makeText(LaifengRtpActivity.this, "获取流信息失败", Toast.LENGTH_SHORT).show();
                    }
                }
            }
        });

        View liveView = inflater.inflate(R.layout.rtp_live_dialog,(ViewGroup) findViewById(R.id.dialog));

        mLiveHost = (Spinner) liveView.findViewById(R.id.livehostspinner);
        mLiveAppId = (EditText) liveView.findViewById(R.id.appId);
        mLiveAlias = (EditText) liveView.findViewById(R.id.alias);
        mLiveVideoBps = (EditText) liveView.findViewById(R.id.videobps);
        mLiveEnableFec = (EditText) liveView.findViewById(R.id.enableFec);
        mLiveEnableNack = (EditText) liveView.findViewById(R.id.enableNack);
        mLiveLog = (Spinner) liveView.findViewById(R.id.livelogspinner);
        mLiveUploadIP = (EditText) liveView.findViewById(R.id.uploadip);
        mLiveUploadUdpPort = (EditText) liveView.findViewById(R.id.uploadudpport);
        mLiveUploadTcpPort = (EditText) liveView.findViewById(R.id.uploadtcpport);
        mLiveUploadHttpPort = (EditText) liveView.findViewById(R.id.uploadhttpport);
        mLiveStreamId = (EditText) liveView.findViewById(R.id.uploadstreamid);
        mLiveMtuSize = (EditText) liveView.findViewById(R.id.mtusize);
        mLiveNormal = (Button) liveView.findViewById(R.id.normalLive);
        mLiveSwitch = (Switch) liveView.findViewById(R.id.switchSetting);
        mLiveMore = (LinearLayout) liveView.findViewById(R.id.moreSettings);
        mLiveDebug = (Button) liveView.findViewById(R.id.debugLive);
        mCreateStream = (Button) liveView.findViewById(R.id.createstream);
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
                String enablefec = mLiveEnableFec.getText().toString();
                String enablenack = mLiveEnableNack.getText().toString();
                if(TextUtils.isEmpty(appId) || TextUtils.isEmpty(alias) || TextUtils.isEmpty(host) || TextUtils.isEmpty(videobps)
                        || TextUtils.isEmpty(enablefec) || TextUtils.isEmpty(enablenack)) {
                    Toast.makeText(LaifengRtpActivity.this, "请输入正确的直播参数", Toast.LENGTH_SHORT).show();
                } else {
//                    mProgressConnecting.setVisibility(View.VISIBLE);
//                    AudioConfiguration.Builder audioBuilder = new AudioConfiguration.Builder();
//                    audioBuilder.setChannelCount(1).setFrequency(48000).setAec(true);
//                    AudioConfiguration audioConfiguration = audioBuilder.build();
//                    mRtpController.setAudioConfiguration(audioConfiguration);
//
//                    VideoConfiguration videoConfiguration = new VideoConfiguration.Builder().setBps(400,CommonUtils.getInt(videobps,1300)).setFps(24).setIfi(1).setSize(360,640).build();
//                    mRtpController.setVideoConfiguration(videoConfiguration);
//                    createStreamAndStart();
//                    SharedPreferences sharedPreferences = getSharedPreferences("liveparams", Context.MODE_PRIVATE);
//                    SharedPreferences.Editor editor = sharedPreferences.edit();
//                    editor.putString("livealias", alias);
//                    editor.commit();
                    liveDialog.dismiss();
                }
            }
        });
        mLiveSwitch.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if(isChecked) {
                    mLiveMore.setVisibility(View.VISIBLE);
                    mLiveNormal.setVisibility(View.GONE);
                } else {
                    mLiveMore.setVisibility(View.GONE);
                    mLiveNormal.setVisibility(View.VISIBLE);
                }
            }
        });

        mCreateStream.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String appId = mLiveAppId.getText().toString();
                String host = mLiveHost.getSelectedItem().toString();
                String alias = mLiveAlias.getText().toString();
                String videobps = mLiveVideoBps.getText().toString();
                if(TextUtils.isEmpty(appId) || TextUtils.isEmpty(alias) || TextUtils.isEmpty(host) || TextUtils.isEmpty(videobps)) {
                    Toast.makeText(LaifengRtpActivity.this, "请输入正确的直播参数", Toast.LENGTH_SHORT).show();
                } else {
//                    mProgressConnecting.setVisibility(View.VISIBLE);
//                    AudioConfiguration.Builder audioBuilder = new AudioConfiguration.Builder();
//                    audioBuilder.setChannelCount(1).setFrequency(48000).setAec(true);
//                    AudioConfiguration audioConfiguration = audioBuilder.build();
//                    mRtpController.setAudioConfiguration(audioConfiguration);
//
//                    VideoConfiguration videoConfiguration = new VideoConfiguration.Builder().setBps(400,CommonUtils.getInt(videobps,1300)).setFps(24).setIfi(1).setSize(360,640).build();
//                    mRtpController.setVideoConfiguration(videoConfiguration);
//                    createStream();
//                    SharedPreferences sharedPreferences = getSharedPreferences("liveparams", Context.MODE_PRIVATE);
//                    SharedPreferences.Editor editor = sharedPreferences.edit();
//                    editor.putString("livealias", alias);
//                    editor.commit();
                }
            }
        });

        mLiveDebug.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String uploadip = mLiveUploadIP.getText().toString();
                String uploadudpport = mLiveUploadUdpPort.getText().toString();
                String uploadtcpport = mLiveUploadTcpPort.getText().toString();
                String uploadhttpport = mLiveUploadHttpPort.getText().toString();
                String uploadstreamid = mLiveStreamId.getText().toString();
                String mtusize = mLiveMtuSize.getText().toString();
                String videobps = mLiveVideoBps.getText().toString();
                String enablefec = mLiveEnableFec.getText().toString();
                String enablenack = mLiveEnableNack.getText().toString();
                if(TextUtils.isEmpty(enablefec) || TextUtils.isEmpty(enablenack) || TextUtils.isEmpty(videobps) ||TextUtils.isEmpty(uploadip)
                         || TextUtils.isEmpty(uploadudpport)|| TextUtils.isEmpty(uploadtcpport)
                         || TextUtils.isEmpty(uploadhttpport)|| TextUtils.isEmpty(mtusize)|| TextUtils.isEmpty(uploadstreamid)) {
                    Toast.makeText(LaifengRtpActivity.this, "请输入正确的直播参数", Toast.LENGTH_SHORT).show();
                } else {
                    mProgressConnecting.setVisibility(View.VISIBLE);
//                    if(mRtpController != null) {
//                        UploadParams uploadParams = new UploadParams(uploadip,uploadudpport,uploadtcpport,uploadhttpport,uploadstreamid,CommonUtils.getInt(enablefec,0),CommonUtils.getInt(enablenack,1));
//                        mRtpController.setUploadParams(uploadParams);
//                        mRtpController.startLiveWithParams();
//                    }
                    liveDialog.dismiss();
                }
            }
        });

    }

    private void createStream() {
        String appId = mLiveAppId.getText().toString();
        String host = mLiveHost.getSelectedItem().toString();
        String alias = mLiveAlias.getText().toString();

        String url = String.format(CREATE_STREAM_URL, host, alias, appId);
        Log.d(TAG, "create stream url : " + url);
        OkHttpClient mOkHttpClient = new OkHttpClient();
        Request request = new Request.Builder()
                .url(url)
                .build();
        Call call = mOkHttpClient.newCall(request);
        call.enqueue(mCreateStreamCallBack);
    }

    private Callback mCreateStreamCallBack = new Callback() {
        @Override
        public void onFailure(Call call, IOException e) {
            final String message = e.getMessage();
            handler.post(new Runnable() {
                @Override
                public void run() {
                    mProgressConnecting.setVisibility(View.GONE);
                    Toast.makeText(LaifengRtpActivity.this, "创建流失败,错误码：" + message, Toast.LENGTH_SHORT).show();
                }
            });
        }

        @Override
        public void onResponse(Call call, final Response response) throws IOException {
            if (response.isSuccessful()) {
                String appId = mLiveAppId.getText().toString();
                String alias = mLiveAlias.getText().toString();
                String host = mLiveHost.getSelectedItem().toString();
                int logLevel = 0;
                if(mLiveLog.getSelectedItem().toString().equalsIgnoreCase("SEVER-LEVEL") == true)
                    logLevel = 0;
                else if(mLiveLog.getSelectedItem().toString().equalsIgnoreCase("CLIENT-NON") == true)
                    logLevel = 1;
                else if(mLiveLog.getSelectedItem().toString().equalsIgnoreCase("CLIENT-RTP") == true)
                    logLevel = 2;
                String deviceid = CommonUtils.getUniquePsuedoID().replaceAll("-","");
                String userid = deviceid.substring(deviceid.length()/2).replaceAll("[^0-9]","");
//                StreamInfo streamInfo = new StreamInfo(appId,"98765",alias,"",Long.valueOf(userid),0);
//                streamInfo.setLapiIp(host);
//                mRtpController.setStreamInfo(streamInfo);
//                mRtpController.setLogLevel(logLevel);
//                if(mRtpController.startConnect()) {
//                    handler.post(new Runnable() {
//                        @Override
//                        public void run() {
//                            UploadParams params = mRtpController.getUploadParams();
//                            if (params != null) {
//                                mLiveUploadIP.setText(params.getUploadIp());
//                                mLiveUploadUdpPort.setText(String.valueOf(params.getUploadUdpPort()));
//                                mLiveUploadTcpPort.setText(String.valueOf(params.getUploadTcpPort()));
//                                mLiveUploadHttpPort.setText(String.valueOf(params.getUploadHttpPort()));
//                                mLiveStreamId.setText(String.valueOf(params.getUploadStreamId()));
//                            }
//                        }
//                    });
//                }
            }
            if(response != null)
                response.body().close();
        }
    };

    private void destroyStream() {
        String appId = mLiveAppId.getText().toString();
        String host = mLiveHost.getSelectedItem().toString();
        String alias = mLiveAlias.getText().toString();

        String url = String.format(DESTROY_STREAM_URL, host, alias, appId);
        Log.d(TAG, "destroy stream url : " + url);
        OkHttpClient mOkHttpClient = new OkHttpClient();
        Request request = new Request.Builder()
                .url(url)
                .build();
        Call call = mOkHttpClient.newCall(request);
        call.enqueue(mDestroyStreamCallBack);
    }

    private Callback mDestroyStreamCallBack = new Callback() {
        @Override
        public void onFailure(Call call, IOException e) {
            final String message = e.getMessage();
            handler.post(new Runnable() {
                @Override
                public void run() {
                    Toast.makeText(LaifengRtpActivity.this, "销毁流失败，错误码:" + message, Toast.LENGTH_SHORT).show();
                }
            });
        }

        @Override
        public void onResponse(Call call, final Response response) throws IOException {
            if(response != null)
                response.body().close();
        }
    };

    private void createStreamAndStart() {
        String appId = mLiveAppId.getText().toString();
        String host = mLiveHost.getSelectedItem().toString();
        String alias = mLiveAlias.getText().toString();

        String url = String.format(CREATE_STREAM_URL, host, alias, appId);
        Log.w(TAG, "create stream url : " + url);
        OkHttpClient mOkHttpClient = new OkHttpClient();
        Request request = new Request.Builder()
                .url(url)
                .build();
        Call call = mOkHttpClient.newCall(request);
        call.enqueue(mCreateStreamAndStartCallBack);
    }

    private Callback mCreateStreamAndStartCallBack = new Callback() {
        @Override
        public void onFailure(Call call, IOException e) {
            final String message = e.getMessage();
            handler.post(new Runnable() {
                @Override
                public void run() {
                    mProgressConnecting.setVisibility(View.GONE);
                    Toast.makeText(LaifengRtpActivity.this, "创建流失败，错误码:" + message, Toast.LENGTH_SHORT).show();
                }
            });
        }

        @Override
        public void onResponse(Call call, final Response response) throws IOException {
            if (response.isSuccessful()) {
                String appId = mLiveAppId.getText().toString();
                String alias = mLiveAlias.getText().toString();
                String host = mLiveHost.getSelectedItem().toString();
                String enablefec = mLiveEnableFec.getText().toString();
                String enablenack = mLiveEnableNack.getText().toString();
                int logLevel = 0;
                if(mLiveLog.getSelectedItem().toString().equalsIgnoreCase("SEVER-LEVEL") == true)
                    logLevel = 0;
                else if(mLiveLog.getSelectedItem().toString().equalsIgnoreCase("CLIENT-NON") == true)
                    logLevel = 1;
                else if(mLiveLog.getSelectedItem().toString().equalsIgnoreCase("CLIENT-RTP") == true)
                    logLevel = 2;
                String deviceid = CommonUtils.getUniquePsuedoID().replaceAll("-","");
                String userid = deviceid.substring(deviceid.length()/2).replaceAll("[^0-9]","");
//                StreamInfo streamInfo = new StreamInfo(appId,"98765",alias,"",Long.valueOf(userid),0);
//                streamInfo.setLapiIp(host);
//                mRtpController.setStreamInfo(streamInfo);
//                mRtpController.setLogLevel(logLevel);
//                mRtpController.startLive();
            } else {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        mProgressConnecting.setVisibility(View.GONE);
                        Toast.makeText(LaifengRtpActivity.this, "创建流失败，错误码:" + response.code(), Toast.LENGTH_SHORT).show();
                    }
                });
            }
            if(response != null)
                response.body().close();
        }
    };

    private synchronized void prepareRtpPlayer() {
        Log.d(TAG, "prepareRtpPlayer.");
        if(mLFPlayer != null)
            return;
        mLFPlayer = new RtpPlayer();
        mLFPlayer.setCallback(mRtpPlayerCallback);
        mLFPlayer.CreatePlayer();
        mLFPlayer.SetWindow(mSurfaceView.getHolder().getSurface());
        if(s1 == str1 + str2) {
            if(mLFPlayer2 != null)
                return;
            mLFPlayer2 = new RtpPlayer();
            mLFPlayer2.setCallback(mRtpPlayerCallback);
            mLFPlayer2.CreatePlayer();
            mLFPlayer2.SetWindow(mSurfaceView2.getHolder().getSurface());
        }
    }

    private synchronized void playRtpStream(boolean isAdvanced) {
        Log.d(TAG, "playRtpStream.");
//        mSurfaceView.setVisibility(View.VISIBLE);
//        if(s1 == str1 + str2)
//            mSurfaceView2.setVisibility(View.VISIBLE);
        //mLFPlayer.StopPlay();
        if(isAdvanced) {
            mLFPlayer.AdvancedStartPlay();
            if(s1 == str1 + str2)
                mLFPlayer2.AdvancedStartPlay();
        }
        else {
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
            mLFPlayer.StartPlay(appId, alias, host,"98765",CommonUtils.getUniquePsuedoID(),null,logLevel);
            if(s1 == str1 + str2) {
                String alias2 = mPlayAlias2.getText().toString();
                mLFPlayer2.StartPlay(appId, alias2, host, "98765",CommonUtils.getUniquePsuedoID(),null,logLevel);
            }
        }
    }

    private RtpPlayer.LFLiveCallback mRtpPlayerCallback = new RtpPlayer.LFLiveCallback() {
        @Override
        public void CallBackMessageFromNative(Object thiz,int msgid,String content, int wParam, int lParam) {
            final int err = msgid;
            RtpPlayer player = (RtpPlayer)thiz;
            if(player == null)
                return;
            int playerIndex = -1;
            if(player == mLFPlayer)
                playerIndex = 1;
            if(player == mLFPlayer2)
                playerIndex = 2;
            if(playerIndex == -1)
                return;
            final int index = playerIndex;

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
                        if(index == 1) {
                            RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams) mSurfaceView.getLayoutParams();
                            Display display = getWindowManager().getDefaultDisplay();
                            Point point = new Point();
                            display.getSize(point);
                            float large_view_width = point.x;
                            lp.width = (int) (large_view_width / 3.0f);
                            lp.height = (int) (lp.width * (height / width));
                            mSurfaceView.setLayoutParams(lp);
                        } else if(index == 2) {
                            RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams) mSurfaceView2.getLayoutParams();
                            Display display = getWindowManager().getDefaultDisplay();
                            Point point = new Point();
                            display.getSize(point);
                            float large_view_width = point.x;
                            lp.width = (int) (large_view_width / 3.0f);
                            lp.height = (int) (lp.width * (height / width));
                            mSurfaceView2.setLayoutParams(lp);
                        }
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

                        if (index == 1) {
                            mSurfaceView.setVisibility(View.VISIBLE);
                        } else if(index == 2) {
                            mSurfaceView2.setVisibility(View.VISIBLE);
                        }
                    }
                });
            }
        }
    };

    private synchronized void stopRtpPlayer() {
        Log.d(TAG, "stopRtpPlayer.");
        if(isPlaying == false)
            return;
        isPlaying = false;
        if(mLFPlayer != null) {
            mLFPlayer.StopPlay();
        }
        if (s1 == str1 + str2 && mLFPlayer2 != null)
            mLFPlayer2.StopPlay();

        handler.post(new Runnable() {
            @Override
            public void run() {
                mSurfaceView.setVisibility(View.GONE);
                if (s1 == str1 + str2) {
                    mSurfaceView2.setVisibility(View.GONE);
                }
                mPlayBtn.setText("播放");
            }
        });
    }

    private void showPlayDialog() {
        mPlayAlias.setText(mLiveAlias.getText());
        SharedPreferences sharedPreferences = getSharedPreferences("playparams", Context.MODE_PRIVATE);
        String playalias = sharedPreferences.getString("playalias", "");
        if(!playalias.isEmpty())
            mPlayAlias.setText(playalias);
        if(s1 == str1 + str2)
            mPlayAlias2.setText(mLiveAlias.getText());
        playDialog.show();
    }

    private void showLiveDialog() {
        liveDialog.show();
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
        destroyStream();
        isRecording = false;
        isPlaying = false;
//        if (mRtpController != null) {
//            mRtpController.stopLive();
//            mRtpController.releaseLive();
//            mRtpController= null;
//        }
        debughandler.removeCallbacks(null);
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
    }

    Runnable debugrunnable = new Runnable() {
        @Override
        public void run() {
            if(isRecording) {
//                try {
//                    mListView.setVisibility(View.VISIBLE);
//                    SendReport sendreport = new SendReport();
//                    sendreport = mRtpController.getSendReport(sendreport);
//                    sendReportList.add(sendreport);
//                    if (sendReportList.size() >= 20) {
//                        sendreport = sendReportList.remove(0);
//                        sendreport = null;
//                    }
//                    myAdapter.notifyDataSetChanged();
//                    if(mLogToScreen) {
//                        mListView.setSelection(mListView.getLastVisiblePosition());
//                        mListView.setTranscriptMode(ListView.TRANSCRIPT_MODE_ALWAYS_SCROLL);
//                    } else {
//                        mListView.setVisibility(View.GONE);
//                    }
//                } catch (Exception e) {
//                    // TODO Auto-generated catch block
//                    e.printStackTrace();
//                }
            }
            debughandler.postDelayed(this, TIME);
        }
    };

//class MyAdapter extends BaseAdapter {
//        @Override
//        public int getCount() {
//            return sendReportList.size();
//        }
//
//        @Override
//        public Object getItem(int position) {
//            if(position >= getCount())
//                return null;
//            return sendReportList.get(position);
//        }
//
//        @Override
//        public long getItemId(int position) {
//            return position;
//        }
//
//        @Override
//        public View getView(int position, View convertView, ViewGroup parent) {
//            if(convertView == null) {
//                convertView = new TextView(LaifengRtpActivity.this);
//            }
//            if(position >= getCount())
//                return null;
//            SendReport sendreport = sendReportList.get(position);
//            String content = "rtt:" + sendreport.rtt_ms + " bitrate:" + sendreport.total_bitrate/1000 + "k target bitrate:" + sendreport.effect_bitrate/1000 + "k lostrate:" + (float)(Math.round(sendreport.packet_lost_rate*1000))/1000;
//            ((TextView)convertView).setText(content);
//            return convertView;
//        }
//    }
}
