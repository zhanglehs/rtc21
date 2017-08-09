1. engineAdapter_sdk工程
    该工程的目的是将engine_sdk的c++的播放器接口重新封装成oc类。
include             // 该目录是对外接口，即整个sdk的最终对外输出。
  LFPlayerCore.h    // 来疯app中集成了多种播放器，为这些播放器制定了统一的接口
  LFRTPPlayerCore.h // 这是我们最终的播放器接口，继承自LFPlayerCore，多提供了两个接口
  LFVideoItem.h     // 来疯制定的统一播放器接口的一部分，看代码就明白了
LFRTPPlayerCore.mm  // 实现来疯的统一播放器
LFVideoItem.m
ReachabilityEngineAdapter.h  // 监测网络变化
ReachabilityEngineAdapter.m
ios_video_render_api.h       // webrtc工程创建render的接口
LFPlayerEngineAdapter.h
LFPlayerEngineAdapter.mm     // 对engine_sdk的封装，主要是对render创建、网络变化监测进行了隐藏

2. LFMixStreamDemo工程
InitViewController.h     // 九宫格View
InitViewController.m
ViewController.h         // 上下播界面
ViewController.m
LFLogTestView.h          // 播放界面的log打印信息View
LFLogTestView.m
LFPlayerTestView.h       // 开始播放时参数设置界面
LFPlayerTestView.m

3. 工程依赖
仅依赖YYKit
在LFMixStreamDemo目录下执行pod install即可（该命令会从https://github.com/zhanglehs/YYKit.git拉取YYKit）。
