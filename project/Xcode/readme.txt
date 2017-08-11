1. engineAdapter_sdk工程
    该工程的目的是将engine_sdk的c++的播放器接口重新封装成oc类。
include             // 该目录是对外接口，即整个sdk的最终对外输出。
  LFPlayerEngineAdapter.h
ReachabilityEngineAdapter.h  // 监测网络变化
ReachabilityEngineAdapter.m
ios_video_render_api.h       // webrtc工程创建render的接口
LFPlayerEngineAdapter.mm     // 对engine_sdk的封装，主要是对render创建、网络变化监测进行了隐藏

2. LFMixStreamDemo工程
MainViewController.h     // 九宫格View，目前该文件没被用到
MainViewController.m
RtpViewController.h      // 采集上传+播放界面
RtpViewController.m
LFPlayerSettingView.h    // 播放界面参数设置View
LFPlayerSettingView.m
LFCaptureSettingView.h   // 采集参数设置View
LFCaptureSettingView.m

3. 工程依赖
仅依赖YYKit
在LFMixStreamDemo目录下执行pod install即可（该命令会从https://github.com/zhanglehs/YYKit.git拉取YYKit）。
