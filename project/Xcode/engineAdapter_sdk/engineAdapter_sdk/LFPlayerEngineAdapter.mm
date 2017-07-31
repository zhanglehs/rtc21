//
//  LFRTPPlayerCore.m
//  LFRTPPlayerCore
//
//  Created by admin on 2016/12/29.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "LFPlayerEngineAdapter.h"
#import "rtp_api.h"
#import "ios_video_render_api.h"
#import "RtcLog.h"
#import "RtcPlayer.h"
#import "ReachabilityEngineAdapter.h"
#import <AVFoundation/AVFoundation.h>

@implementation LFRtpPlayerParam
@end

@interface LFPlayerEngineAdapter ()
@property (nonatomic, strong) LFRtpPlayerParam *mPlayerParam;
@property (nonatomic, strong) UIView *mVideoView;
@property (nonatomic, assign) void *mVideoHnd;
@property (nonatomic, strong) ReachabilityEngineAdapter *mNetReachability;
@property (nonatomic, assign) RtcPlayer *mPlayer;
@end

@implementation LFPlayerEngineAdapter
    
-(int)init:(LFRtpPlayerParam *)param
{
    _mPlayerParam = param;
    
    void *viewHnd = NULL;
    int scale = [UIScreen mainScreen].scale;
    void *videoHandle = lfrtcCreateIosVideoRender(65535,
        CGRectMake(0, 0, _mPlayerParam.videoSize.width, _mPlayerParam.videoSize.height),scale,NULL, &viewHnd);
    _mVideoView = (__bridge UIView*)videoHandle;
    self.mVideoHnd = viewHnd;
    UIApplicationState state = [UIApplication sharedApplication].applicationState;
    if(self.mVideoHnd && (UIApplicationStateActive != state)) {
        CREATE_VIDEO_RENDER_PAUSE(self.mVideoHnd,true);
    }
    _mVideoView.backgroundColor = [UIColor blackColor];
    _mVideoView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    
    _mPlayer = new RtcPlayer("xxxdeviceid_iosxxx");
    _mPlayer->SetUserdata((__bridge void*)self);
    
    [self installNotifications];
    
    _mNetReachability = [ReachabilityEngineAdapter reachabilityForInternetConnection];
    _mNetReachability.reachableBlock = ^(ReachabilityEngineAdapter * ){
        if([_mNetReachability isReachable]){
            _mPlayer->SetNetworkChanged();
        }
    };
    
    return 0;
}

-(int)uninit
{
    self.mDelegate = nil;
    [self removeNotifications];
    [self stopPlay];
    lfrtcDestroyIosVideoRender(self.mVideoHnd);
    self.mVideoHnd = nil;
    delete _mPlayer;
    _mPlayer = nil;
    return 0;
}

-(int)startPlay
{
    if(self.mDelegate && [self.mDelegate respondsToSelector:@selector(onPlayerStartLoading:msg:)]){
        [self.mDelegate onPlayerStartLoading:0 msg:@"播放初始中"];
    }
    
    if(_mPlayerParam && _mPlayerParam.appId && _mPlayerParam.alias && _mPlayerParam.token){
        RtcPlayer::NetworkConfig config;
        strcpy(config.alias, [_mPlayerParam.alias UTF8String]);
        strcpy(config.appid, [_mPlayerParam.appId UTF8String]);
        strcpy(config.lapi, [_mPlayerParam.lapiUrl UTF8String]);
        strcpy(config.token, [_mPlayerParam.token UTF8String]);
        _mPlayer->Start(&config, eventNotifyRTPOnState);
        _mPlayer->SetWindow(self.mVideoHnd);
        [_mNetReachability startNotifier];
    }
    return 0;
}

-(int)stopPlay
{
    _mPlayer->Stop();
    [_mNetReachability stopNotifier];
    return 0;
}

-(int)pausePlay
{
    return 0;
}

-(int)resumePlay
{
    return 0;
}

-(int)setPlayerEventListenerDelegate:(id< LFRtpPlayerEventDelegate > _Nullable)delegate
{
    _mDelegate = delegate;
    return 0;
}

-(int)snapShot:(NSString *)var1
{
    return _mPlayer->Snapshot([var1 UTF8String]);
}

-(int)setMuted:(BOOL)mute
{
    _mPlayer->Mute((bool)mute);
    return 0;
}

-(nullable UIView*)renderView
{
    return _mVideoView;
}

-(BOOL)isPlaying
{
    RtcPlayerState state =_mPlayer->GetState();
    return state != RTC_PLAYER_STATE_UNSET && state != RTC_PLAYER_STATE_STOPPED;
}

-(int)getVideoWidth
{
    return 0;
}

-(int)getVideoHeight
{
    return 0;
}

- (void)dealloc
{
    [self uninit];
}

static void eventNotifyRTPOnState(RtcPlayer* player, int msgid, long wParam, long lParam) {
    NSLog(@"message_callback msgid: %d, %d, %d\n",msgid, (int)wParam, (int)lParam);
    LFPlayerEngineAdapter *playerEngine = (__bridge LFPlayerEngineAdapter*)player->GetUserdata();
    if (playerEngine && playerEngine.mDelegate && [playerEngine.mDelegate respondsToSelector:@selector(onPlayerError:msg:)]){
        if (msgid == RTC_PLAYER_STATE_ERROR) {
            [playerEngine.mDelegate onPlayerError:msgid
                                               msg:@"播放错误"];
        }
        else if (msgid == RTC_PLAYER_MSG_VIDEO_RESOLUTION) {
            [playerEngine.mDelegate onPlayerDecodeVideoSize:(int)wParam
                                                     height:(int)lParam];
        }
        else if (msgid == RTC_PLAYER_STATE_INITIALIZING) {
            [playerEngine.mDelegate onPlayerStartLoading:msgid
                                                msg:@"播放初始中"];
        }
        else if (msgid == RTC_PLAYER_MSG_VIDEO_FIRST_FRAME) {
            [playerEngine.mDelegate onPlayerFirstPicture:msgid
                                                     msg:@"视频首帧出现"];
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                if([[AVAudioSession sharedInstance] respondsToSelector:@selector(setCategory:mode:options:error:)]){
                    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayAndRecord
                                                            mode:AVAudioSessionModeVoiceChat
                                                         options:AVAudioSessionCategoryOptionDefaultToSpeaker
                                                           error:nil];
                    [[AVAudioSession sharedInstance] setActive:YES error:nil];
                }
                else{
                    [[AVAudioSession sharedInstance] setMode:AVAudioSessionModeVoiceChat
                                                       error:nil];
                    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayAndRecord
                                                     withOptions:AVAudioSessionCategoryOptionDefaultToSpeaker
                                                           error:nil];
                    [[AVAudioSession sharedInstance] setActive:YES error:nil];
                }
            });
        }
    }
}

- (void)installNotifications
{
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillEnterForeground) name:UIApplicationWillEnterForegroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive) name:UIApplicationDidBecomeActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive) name:UIApplicationWillResignActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidEnterBackground) name:UIApplicationDidEnterBackgroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillTerminate) name:UIApplicationWillTerminateNotification object:nil];
}

- (void)removeNotifications
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark -- application
- (void)applicationWillEnterForeground
{
}

- (void)applicationDidBecomeActive
{
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        if([[AVAudioSession sharedInstance] respondsToSelector:@selector(setCategory:mode:options:error:)]){
            [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayAndRecord
                                                    mode:AVAudioSessionModeVoiceChat
                                                 options:AVAudioSessionCategoryOptionDefaultToSpeaker
                                                   error:nil];
            [[AVAudioSession sharedInstance] setActive:YES error:nil];
        }
        else{
            [[AVAudioSession sharedInstance] setMode:AVAudioSessionModeVoiceChat
                                               error:nil];
            [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayAndRecord
                                             withOptions:AVAudioSessionCategoryOptionDefaultToSpeaker
                                                   error:nil];
            [[AVAudioSession sharedInstance] setActive:YES error:nil];
        }
    });
    [self resumePlay];
    if(self.mVideoHnd){
        CREATE_VIDEO_RENDER_PAUSE(self.mVideoHnd,false);
    }
}

- (void)applicationWillResignActive
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategorySoloAmbient
                                               error:nil];
        [[AVAudioSession sharedInstance] setActive:YES error:nil];
    });
    [self pausePlay];
    if(self.mVideoHnd){
        CREATE_VIDEO_RENDER_PAUSE(self.mVideoHnd,true);
    }
}

- (void)applicationDidEnterBackground
{
}

- (void)applicationWillTerminate
{
}

@end
