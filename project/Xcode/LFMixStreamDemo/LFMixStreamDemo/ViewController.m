//
//  ViewController.m
//  LFMixStreamDemo
//
//  Created by admin on 16/9/12.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "ViewController.h"
#import <YYKit/YYKit.h>
#import "LFSessionTestView.h"
#import "LFPlayerTestView.h"
#import "../../engineAdapter_sdk/engineAdapter_sdk/include/LFRTPPlayerCore.h"
#import "../../engineAdapter_sdk/engineAdapter_sdk/include/LFPlayerEngineAdapter.h"
#import "LFLogTestView.h"
#import <YYKit/YYReachability.h>
#import <AdSupport/ASIdentifierManager.h>

@interface ViewController ()<LFPlayerDelegate>

@property (nonatomic, strong) UIButton *leftButton;
@property (nonatomic, strong) UIButton *rightButton;
@property (nonatomic, strong) UIButton *cameraButton;
@property (nonatomic, strong) UIButton *logButton;
@property (nonatomic, strong) UIView *playerView;
@property (nonatomic, strong) LFSessionTestView *sessionTestView;
@property (nonatomic, strong) LFPlayerTestView *playerTestView;
@property (nonatomic, strong) LFLogTestView *logView;
@property (nonatomic, strong) id<LFPlayerCore> player;
#ifdef MULTIPLAYER
@property (nonatomic, strong) id<LFPlayerCore> player2;
@property (nonatomic, strong) UIView *player2View;
#endif
@property (nonatomic, assign) BOOL isPlaying;
@property (nonatomic, strong) rtcCaptureAdapter* capturer;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    CGSize capture_size = self.view.bounds.size;
    capture_size.width /=2;
    capture_size.height /=2;
    _capturer = [[rtcCaptureAdapter alloc] initWith:&capture_size];
    UIView *capture_view = [_capturer renderView];
    [capture_view setHidden:TRUE];
    
    [self.view addSubview:self.playerView];
#ifdef MULTIPLAYER
    [self.view addSubview:self.player2View];
#endif
    [self.view addSubview:self.logView];
    [self.view addSubview:self.cameraButton];
    [self.view addSubview:self.logButton];
    [self.view addSubview:capture_view];
    [self.view addSubview:self.sessionTestView];
    [self.view addSubview:self.playerTestView];
    [self.view addSubview:self.leftButton];
    [self.view addSubview:self.rightButton];
    self.view.backgroundColor = [UIColor grayColor];
}

- (void)viewDidAppear:(BOOL)animated{
    [super viewDidAppear:animated];
    [UIApplication sharedApplication].idleTimerDisabled = YES;
}

- (void)viewDidDisappear:(BOOL)animated{
    [super viewDidDisappear:animated];
    [UIApplication sharedApplication].idleTimerDisabled = NO;
}

#pragma mark --player api
- (void)startRtpPlayer{
    LFVideoItem *item = [LFVideoItem new];
    item.appId = self.playerTestView.appid;
    item.alias = self.playerTestView.alias;
    item.hosturl = self.playerTestView.hosturl;
    item.token = @"98765";
    item.videoSize = self.playerView.size;
    self.player = [LFRTPPlayerCore alloc];
    [self.player setDelegate:self];
    [self.player initWithVideoItem:item];
    [self.playerView addSubview:self.player.renderView];
    self.player.renderView.frame = self.playerView.bounds;
    self.player.renderView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    dispatch_async(dispatch_get_main_queue(), ^{
        self.playerView.hidden = YES;
    });
    
    [self.player prepareToPlay];
}

-(void) stopRtpPlayer{
    [self.player stop];
    self.player = nil;
    self.isPlaying = NO;
}

#pragma mark --live api

#pragma mark -- Setter Getter
- (UIButton*)leftButton{
    if(!_leftButton){
        _leftButton = [UIButton new];
        _leftButton.size = CGSizeMake(60, 30);
        _leftButton.left = 10;
        _leftButton.top = 30;
        [_leftButton setTitle:@"开播" forState:UIControlStateNormal];
        _leftButton.accessibilityIdentifier = @"buttonKaiBo";
        @weakify(self)
        [_leftButton addBlockForControlEvents:UIControlEventTouchUpInside block:^(id  _Nonnull sender) {
            @strongify(self)
            [self.view.window endEditing:YES];
            self.playerTestView.left = self.view.width;
            [UIView animateWithDuration:0.3 animations:^{
                if(self.sessionTestView.left == 0) self.sessionTestView.left = self.view.width * -1;
                else self.sessionTestView.left = 0;
            }];
            //if(self.isLiving) {
            //    [self.session stopLive];
            //    [self.sessionTestView destroyStream:self.sessionTestView.appid alias:self.sessionTestView.alias urlHost:self.sessionTestView.host];
            //    self.session = nil;
            //}
            [self.leftButton setTitle:@"开播" forState:UIControlStateNormal];
            //self.isLiving = NO;
        }];
        [_leftButton setBackgroundColor:[UIColor greenColor]];
    }
    return _leftButton;
}

- (UIButton*)rightButton{
    if(!_rightButton){
        _rightButton = [UIButton new];
        _rightButton.size = CGSizeMake(60, 30);
        _rightButton.right = self.view.width - 10;
        _rightButton.top = 30;
        [_rightButton setTitle:@"播放" forState:UIControlStateNormal];
        _rightButton.accessibilityIdentifier = @"buttonBoFang";
        @weakify(self)
        [_rightButton addBlockForControlEvents:UIControlEventTouchUpInside block:^(id  _Nonnull sender) {
            @strongify(self)
            [self.view.window endEditing:YES];
            self.sessionTestView.left = self.view.width*-1;
            [UIView animateWithDuration:0.3 animations:^{
                if(self.playerTestView.left == 0) self.playerTestView.left = self.view.width * 1;
                else self.playerTestView.left = 0;
            }];
            if(self.isPlaying){
                dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT,0), ^{
                    [self stopRtpPlayer];
                });
            }
            
            [self.rightButton setTitle:@"播放" forState:UIControlStateNormal];
        }];
        [_rightButton setBackgroundColor:[UIColor greenColor]];
    }
    return _rightButton;
}

- (UIButton*)cameraButton{
    if(!_cameraButton){
        _cameraButton = [UIButton new];
        _cameraButton.size = CGSizeMake(44, 44);
        _cameraButton.left = self.leftButton.right + 40;
        _cameraButton.top = 30;
        [_cameraButton setImage:[UIImage imageNamed:@"camra_preview"] forState:UIControlStateNormal];
    }
    return _cameraButton;
}

- (UIButton*)logButton{
    if(!_logButton){
        _logButton = [UIButton new];
        _logButton.size = CGSizeMake(44, 44);
        _logButton.right = self.rightButton.left - 40;
        _logButton.top = 30;
        [_logButton setTitle:@"Log" forState:UIControlStateNormal];
        @weakify(self)
        [_logButton addBlockForControlEvents:UIControlEventTouchUpInside block:^(id sender) {
            @strongify(self)
            self.logView.hidden = !self.logView.hidden;
        }];
    }
    return _logButton;
}

- (UIView*)playerView{
    if(!_playerView){
        _playerView = [UIView new];
        _playerView.size = CGSizeMake(150, 266);
        _playerView.right = self.view.width;
        _playerView.bottom = self.view.bottom;
    }
    return _playerView;
}
#ifdef MULTIPLAYER
- (UIView*)player2View{
    if(!_player2View){
        _player2View = [UIView new];
        _player2View.size = CGSizeMake(150, 266);
        _player2View.right = self.view.width;
        _player2View.bottom = self.view.bottom - 266;
    }
    return _player2View;
}
#endif

- (LFSessionTestView*)sessionTestView{
    if(!_sessionTestView){
        _sessionTestView = [[LFSessionTestView alloc] initWithFrame:self.view.bounds];
        _sessionTestView.left = self.view.width*-1;
        _sessionTestView.backgroundColor = [UIColor lightGrayColor];

        @weakify(self)
        _sessionTestView.rtpStartBlock = ^(NSString *appid , NSString *alias, NSString *url, int logLevel){
            @strongify(self)
            [UIView animateWithDuration:0.3 animations:^{
                self.sessionTestView.left = self.view.width * -1;
            }];
            
            
            int ret = [self.capturer startCapture:nil];
            ret = [self.capturer StartPreview];
            [[self.capturer renderView] setHidden:FALSE];
            
            rtcOcNetworkConfig *net = [[rtcOcNetworkConfig alloc] init];
            net.lapi = url;
            net.appid = appid;
            net.alias = alias;
            net.token = @"98765";
            [self.capturer StartEncode:nil AndSend:net];
            
            self.playerTestView.appid = appid;
            self.playerTestView.alias = alias;
#ifdef MULTIPLAYER
            self.playerTestView.alias2 = alias;
#endif
            [self.leftButton setTitle:@"停止" forState:UIControlStateNormal];
        };
#if 0
        _sessionTestView.rtpInitBlock = ^(NSString *appid , NSString *alias, NSString *url, int logLevel){
            @strongify(self)
            LFRtpLiveStreamInfo *info = [LFRtpLiveStreamInfo new];
            info.appId = appid;
            info.alias = alias;
            info.url = url;
            info.token = @"98765";
            info.logLevel = 0;
            info.uid = [[[ASIdentifierManager sharedManager] advertisingIdentifier] UUIDString];
            self.playerTestView.appid = appid;
            self.playerTestView.alias = alias;
            LiveRtpConstant *globalInfo = [self globalinfoSetting];
            globalInfo.UPLOAD_CONNECT_TIMEOUT_MS = 300*1000;
            globalInfo.UPLOAD_RECONNECT_TIMEOUT_MS = 600*1000;
            [self.session updateGlobalInfo:globalInfo];
            return [self.session initLive:info];
        };

        _sessionTestView.rtpStartBlockWithExtraParams = ^(NSString *uploadIp, NSString *uploadUdpPort,NSString *uploadTcpPort,NSString *uploadHttpPort,NSString *uploadStreamId,NSString *mtusize,NSString *enablefec,NSString *enablenack){
            @strongify(self)
            [UIView animateWithDuration:0.3 animations:^{
                self.sessionTestView.left = self.view.width * -1;
            }];
            [self.session setUploadParams:uploadIp
                                  udpport:uploadUdpPort
                                  tcpport:uploadTcpPort
                                 httpport:uploadHttpPort
                                 streamid:uploadStreamId
                                  mtusize:[mtusize intValue]
                                enablefec:[enablefec intValue]
                               enablenack:[enablenack intValue]];
            BOOL ret = [self.session startLiveWithParams];
            if(ret == YES) {
                [self.leftButton setTitle:@"停止" forState:UIControlStateNormal];
                self.isLiving = YES;
            }
        };
        _sessionTestView.getUploadIp = ^ (void) {
            @strongify(self)
            if([self.session isKindOfClass:[LFRtpSession class]]){
                return [self.session getUploadIp];
            }
            return @"";
        };
        _sessionTestView.getUploadUdpPort = ^ (void) {
            @strongify(self)
            if([self.session isKindOfClass:[LFRtpSession class]]){
                return [self.session getUploadUdpPort];
            }
            return 0;
        };
        _sessionTestView.getUploadTcpPort = ^ (void) {
            @strongify(self)
            if([self.session isKindOfClass:[LFRtpSession class]]){
                return [self.session getUploadTcpPort];
            }
            return 0;
        };
        _sessionTestView.getUploadHttpPort = ^ (void) {
            @strongify(self)
            if([self.session isKindOfClass:[LFRtpSession class]]){
                return [self.session getUploadHttpPort];
            }
            return 0;
        };
        _sessionTestView.getUploadStreamId = ^ (void) {
            @strongify(self)
            if([self.session isKindOfClass:[LFRtpSession class]]){
                return [self.session getUploadStreamId];
            }
            return @"";
        };
#endif
    }
    return _sessionTestView;
}

- (LFPlayerTestView*)playerTestView{
    if(!_playerTestView){
        _playerTestView = [[LFPlayerTestView alloc] initWithFrame:self.view.bounds];
        _playerTestView.left = self.view.width*1;
        _playerTestView.backgroundColor = [UIColor lightGrayColor];
        @weakify(self)
        _playerTestView.startRtpBlock = ^(NSString *appid, NSString *alias, NSString *hosturl,int logLevel){
            @strongify(self)
            [UIView animateWithDuration:0.3 animations:^{
                self.playerTestView.left = self.view.width * 1;
            }];
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                [self startRtpPlayer];
                self.isPlaying = YES;
            });
            [self.rightButton setTitle:@"停止" forState:UIControlStateNormal];
        };
    }
    return _playerTestView;
}

- (LFLogTestView*)logView{
    if(!_logView){
        _logView = [[LFLogTestView alloc] initWithFrame:self.view.bounds];
    }
    return _logView;
}

- (void)resize:(CGSize)size{
    if(!self.playerView) {
        return;
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.player.renderView removeFromSuperview];
        int right = self.playerView.right;
        int bottom = self.playerView.bottom;
        self.playerView.size = size;
        self.playerView.right = right;
        self.playerView.bottom = bottom;
        self.player.renderView.size = size;
        [self.playerView addSubview:self.player.renderView];
    });

}

#pragma mark -- Notify
//< 底层播放器向上层通知
- (void)playerNotify:(LFPlayerCode)code{
    switch (code) {
        case LFPlayerUnknownCode:{
            NSLog(@"LFPlayerUnknownCode :[%lu]\n",(unsigned long)LFPlayerUnknownCode);
        }
            break;
        case LFPlayerPlayCompletedCode:
            NSLog(@"LFPlayerPlayCompletedCode :[%lu]\n",(unsigned long)LFPlayerPlayCompletedCode);
            break;
        case LFPlayerPlayStartLoadingCode:
            NSLog(@"LFPlayerPlayStartLoadingCode :[%lu]\n",(unsigned long)LFPlayerPlayStartLoadingCode);
            break;
        case LFPlayerPlayEndLoadingCode:
            NSLog(@"LFPlayerPlayEndLoadingCode :[%lu]\n",(unsigned long)LFPlayerPlayEndLoadingCode);
            break;
        case LFPlayerPlayErrorCode:{
            NSLog(@"LFPlayerPlayErrorCode :[%lu]\n",(unsigned long)LFPlayerPlayErrorCode);
            dispatch_async(dispatch_get_main_queue(), ^{
                [self stopRtpPlayer];
                UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"播放连接失败,已停止" message:[NSString stringWithFormat:@"%lu",(unsigned long)code] delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
                [alertView show];
            });
        }
            break;
        case LFPlayerSeekStartCode:
            NSLog(@"LFPlayerSeekStartCode :[%lu]\n",(unsigned long)LFPlayerSeekStartCode);
            break;
        case LFPlayerSeekStopCode:
            NSLog(@"LFPlayerSeekStopCode :[%lu]\n",(unsigned long)LFPlayerSeekStopCode);
            break;
        case LFPlayerStartPlayCode:
            NSLog(@"LFPlayerStartPlayCode :[%lu]\n",(unsigned long)LFPlayerStartPlayCode);
            break;
        case LFPlayerPausePlayCode:
            NSLog(@"LFPlayerPausePlayCode :[%lu]\n",(unsigned long)LFPlayerPausePlayCode);
            break;
        case LFPlayerStopPlayCode:
            NSLog(@"LFPlayerStopPlayCode :[%lu]\n",(unsigned long)LFPlayerStopPlayCode);
            break;
        case LFPlayerFirstVideoFrameCode:{
            NSLog(@"LFPlayerFirstVideoFrameCode :[%lu]\n",(unsigned long)LFPlayerFirstVideoFrameCode);
            __weak typeof(self) _self = self;
            dispatch_async(dispatch_get_main_queue(), ^{
                _self.playerView.hidden = NO;
            });
        }
            break;
        default:
            break;
    }
}


- (void)playerNaturalSize:(CGSize)size
{
    CGSize size_tmp = CGSizeMake(size.width * 2/5, size.height *2/5);
    [self resize:size_tmp];
}

- (void)playBackTime:(NSTimeInterval)playBackTime
{
}

- (void)playableDuration:(NSTimeInterval)playableDuration
{
}

- (void)playerSize:(uint64_t)size timestamp:(uint64_t)timestamp type:(int)type{
}

@end
