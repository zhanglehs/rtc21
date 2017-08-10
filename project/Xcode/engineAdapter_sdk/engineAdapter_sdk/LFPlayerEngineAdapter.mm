//
//  LFRTPPlayerCore.m
//  LFRTPPlayerCore
//
//  Created by admin on 2016/12/29.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "include/LFPlayerEngineAdapter.h"
#import "rtp_api.h"
#import "ios_video_render_api.h"
#import "RtcLog.h"
#import "RtcPlayer.h"
#import "RtcCapture.h"
#import "ReachabilityEngineAdapter.h"
#import <AVFoundation/AVFoundation.h>

@interface rtcPlayerAdapter ()
@property (nonatomic, strong) UIView *mVideoView;
@property (nonatomic, assign) void *mVideoHnd;
@property (nonatomic, strong) ReachabilityEngineAdapter *mNetReachability;
@property (nonatomic, assign) RtcPlayer *mPlayer;
@property (nonatomic, weak) id<rtcPlayerEventDelegate> _Nullable mDelegate;
@end

@implementation rtcPlayerAdapter
    
-(id _Nonnull)init
{
    self = [super init];
    if (self) {
        void *viewHnd = NULL;
        int scale = [UIScreen mainScreen].scale;
        void *videoHandle = lfrtcCreateIosVideoRender(65535,
                                                      CGRectMake(0, 0, 360, 640),scale,NULL, &viewHnd);
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
    }
    return self;
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

-(int)startPlay:(rtcOcNetworkConfig* _Nullable)net
{
    if (net && net.lapi && net.appid && net.alias && net.token) {
        RtcPlayer::NetworkConfig config;
        strcpy(config.alias, [net.alias UTF8String]);
        strcpy(config.appid, [net.appid UTF8String]);
        strcpy(config.lapi, [net.lapi UTF8String]);
        strcpy(config.token, [net.token UTF8String]);
        if (_mPlayer->Start(&config, eventNotifyRTPOnState) >= 0
            && _mPlayer->SetWindow(self.mVideoHnd) >= 0) {
            [_mNetReachability startNotifier];
            return 0;
        }
    }
    return -1;
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

-(int)setEventDelegate:(id<rtcPlayerEventDelegate> _Nullable)delegate
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

-(void)dealloc
{
    [self uninit];
}

static void eventNotifyRTPOnState(RtcPlayer* player, int msgid, long wParam, long lParam) {
    NSLog(@"message_callback msgid: %d, %d, %d\n",msgid, (int)wParam, (int)lParam);
    
    rtcPlayerAdapter *pThis = (__bridge rtcPlayerAdapter*)(player->GetUserdata());
    id<rtcPlayerEventDelegate> delegate = pThis.mDelegate;
    if (delegate == nil) {
        return;
    }
    
    switch (msgid) {
        case RTC_PLAYER_MSG_VIDEO_RESOLUTION:
            if ([delegate respondsToSelector:@selector(onPlayerVideoWidth:AndHeight:)]) {
                [delegate onPlayerVideoWidth:(int)wParam AndHeight:(int)lParam];
            }
            break;
        case RTC_PLAYER_MSG_VIDEO_FIRST_FRAME:
            if ([delegate respondsToSelector:@selector(onPlayerFirstPicture)]) {
                [delegate onPlayerFirstPicture];
            }
            
            // TODO: zhangle, should be first audio
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
            break;
        case RTC_PLAYER_STATE_INITIALIZING:
            if ([delegate respondsToSelector:@selector(onPlayerStateInitializing)]) {
                [delegate onPlayerStateInitializing];
            }
            break;
        case RTC_PLAYER_STATE_RUNNING:
            if ([delegate respondsToSelector:@selector(onPlayerStatePlaying)]) {
                [delegate onPlayerStatePlaying];
            }
            break;
        case RTC_PLAYER_STATE_ERROR:
            if ([delegate respondsToSelector:@selector(onPlayerStateRetry)]) {
                [delegate onPlayerStateRetry];
            }
            break;
        case RTC_PLAYER_STATE_STOPPED:
            if (lParam == RTC_PLAYER_STOP_BY_ERROR && [delegate respondsToSelector:@selector(onPlayerStateError)]) {
                [delegate onPlayerStateError];
            }
            break;
            
        default:
            break;
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


/////////////////

@implementation rtcOcCaptureConfig
-(id _Nonnull)init
{
    self = [super init];
    if (self) {
        _audio_deviceid = @"";
        _audio_frequence = 48000;
        _video_deviceid = @"";
        _video_capture_width = 640;
        _video_capture_height = 480;
    }
    return self;
}
@end

@implementation rtcOcEncodeConfig
-(id _Nonnull)init
{
    self = [super init];
    if (self) {
        _video_encode_width = 360;
        _video_encode_height = 640;
        _video_max_fps = 15;
        _video_gop = 15;
        _video_mtu_size = 1200;
        _video_bitrate = 800 * 1024;
    }
    return self;
}
@end

@implementation rtcOcNetworkConfig
@end

@interface rtcCaptureAdapter ()
@property (nonatomic, strong) UIView *mVideoView;
@property (nonatomic, assign) void *mVideoHnd;
//@property (nonatomic, strong) ReachabilityEngineAdapter *mNetReachability;
@property (nonatomic, assign) RtcCapture *mCapture;
@end

@implementation rtcCaptureAdapter
-(id _Nonnull)init
{
    self = [super init];
    if (self) {
        _mCapture = new RtcCapture("xxxdeviceid_iosxxx");
        _mCapture->SetUserdata((__bridge void*)self);

        void *viewHnd = NULL;
        int scale = [UIScreen mainScreen].scale;
        void *videoHandle = lfrtcCreateIosVideoRender(65535,
                                                      CGRectMake(0, 0, 360, 640),scale,NULL, &viewHnd);
        _mVideoView = (__bridge UIView*)videoHandle;
        _mVideoHnd = viewHnd;
        UIApplicationState state = [UIApplication sharedApplication].applicationState;
        if(self.mVideoHnd && (UIApplicationStateActive != state)) {
            CREATE_VIDEO_RENDER_PAUSE(self.mVideoHnd, true);
        }
        _mVideoView.backgroundColor = [UIColor blackColor];
        _mVideoView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillEnterForeground)
                                                     name:UIApplicationWillEnterForegroundNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive)
                                                     name:UIApplicationDidBecomeActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillResignActive)
                                                     name:UIApplicationWillResignActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidEnterBackground)
                                                     name:UIApplicationDidEnterBackgroundNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillTerminate)
                                                     name:UIApplicationWillTerminateNotification object:nil];
    }
    return self;
}

- (void)dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    _mCapture->Stop();
    lfrtcDestroyIosVideoRender(_mVideoHnd);
    delete _mCapture;
}

-(nullable UIView*)renderView
{
    return _mVideoView;
}

-(int)startCapture:(rtcOcCaptureConfig* _Nullable)config
{
    lfrtcCaptureConfig capture_config;
    if(config){
        if (config.audio_deviceid) {
            strcpy(capture_config.audio_deviceid, [config.audio_deviceid UTF8String]);
        }
        capture_config.audio_frequence = config.audio_frequence;
        if (config.video_deviceid) {
            strcpy(capture_config.video_deviceid, [config.video_deviceid UTF8String]);
        }
        capture_config.video_capture_width = config.video_capture_width;
        capture_config.video_capture_height = config.video_capture_height;
    }
    return _mCapture->StartCapture(&capture_config);
}

-(int)StartPreview
{
    return _mCapture->StartPreview(_mVideoHnd);
}

-(int)StartEncode:(rtcOcEncodeConfig* _Nullable)encode AndSend:(rtcOcNetworkConfig* _Nonnull)net
{
    if (net == nil || net.lapi == nil || net.appid == nil || net.alias == nil || net.token == nil) {
        return -1;
    }
    RtcCapture::NetworkConfig net_config;
    strcpy(net_config.lapi, [net.lapi UTF8String]);
    strcpy(net_config.appid, [net.appid UTF8String]);
    strcpy(net_config.alias, [net.alias UTF8String]);
    strcpy(net_config.token, [net.token UTF8String]);
    lfrtcEncodeConfig encode_config;
    if (encode) {
        encode_config.video_encode_width = encode.video_encode_width;
        encode_config.video_encode_height = encode.video_encode_height;
        encode_config.video_max_fps = encode.video_max_fps;
        encode_config.video_gop = encode.video_gop;
        encode_config.video_mtu_size = encode.video_mtu_size;
        encode_config.video_bitrate = encode.video_bitrate;
    }

    // INFO: zhangle, create stream is test code
    char streamid[256] = { 0 };
    char url[1024];
    sprintf(url, "http://%s/v1/create_stream?app_id=%s&alias=%s&stream_type=rtp&res=%dx%d&rt=400&stream_format=rtp&nt=1&token=98765&p2p=0",
               net_config.lapi, net_config.appid, net_config.alias, encode_config.video_encode_width, encode_config.video_encode_height);
    INF("%s, create stream, url=%s", __FUNCTION__, url);
    create_stream_sync(streamid, url);
    INF("%s, create stream finished", __FUNCTION__);

    return _mCapture->StartEncodeAndSend(&net_config, &encode_config);
}

-(void)StopEncodeAndSend
{
    _mCapture->StopEncodeAndSend();
}

-(void)StopPreview
{
    _mCapture->StopPreview();
}

-(void)Stop
{
    _mCapture->Stop();
}

#pragma mark -- application
- (void)applicationWillEnterForeground
{
}

- (void)applicationDidBecomeActive
{
    if (self.mVideoHnd){
        CREATE_VIDEO_RENDER_PAUSE(self.mVideoHnd, false);
    }
}

- (void)applicationWillResignActive
{
    if (self.mVideoHnd){
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
