//
//  ViewController.m
//  LFMixStreamDemo
//
//  Created by admin on 16/9/12.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "ViewController.h"
#import <YYKit/YYKit.h>
#import "LFCaptureSettingView.h"
#import "LFPlayerSettingView.h"
#import "../../engineAdapter_sdk/engineAdapter_sdk/include/LFPlayerEngineAdapter.h"
#import <YYKit/YYReachability.h>
#import <AdSupport/ASIdentifierManager.h>

@interface ViewController ()<rtcPlayerEventDelegate>

@property (nonatomic, strong) UIButton *mCaptureButton;
@property (nonatomic, strong) UIButton *mPlayerButton;
@property (nonatomic, strong) UIButton *cameraButton;
@property (nonatomic, strong) UIButton *logButton;
@property (nonatomic, strong) LFCaptureSettingView *mCaptureSettingView;
@property (nonatomic, strong) LFPlayerSettingView *mPlayerSettingView;
@property (nonatomic, strong) rtcPlayerAdapter *mPlayer;
@property (nonatomic, strong) rtcCaptureAdapter* mCapturer;

@end

@implementation ViewController

-(id _Nonnull)init
{
    self = [super init];
    if (self) {
        self.mCapturer = [[rtcCaptureAdapter alloc] init];
        self.mPlayer = [[rtcPlayerAdapter alloc] init];
        [self.mPlayer setEventDelegate:self];
    }
    return self;
}

- (void)dealloc
{
    [self.mCapturer Stop];
    [self.mPlayer stopPlay];
    self.mCapturer = nil;
    self.mPlayer = nil;
}

- (void)viewDidLoad {
    [super viewDidLoad];

    [self.view addSubview:self.cameraButton];
    [self.view addSubview:self.logButton];
    [self.view addSubview:self.mCaptureSettingView];
    [self.view addSubview:self.mPlayerSettingView];
    [self.view addSubview:self.mCaptureButton];
    [self.view addSubview:self.mPlayerButton];
    self.view.backgroundColor = [UIColor grayColor];
    
    UIView *capture_view = [self.mCapturer renderView];
    [capture_view setFrame:self.view.bounds];
    [capture_view setHidden:TRUE];
    [self.view addSubview:capture_view];
    [self.view sendSubviewToBack:capture_view];
    
    UIView *player_view = [self.mPlayer renderView];
    player_view.size = CGSizeMake(150, 266);
    player_view.right = self.view.width;
    player_view.bottom = self.view.bottom;
    [player_view setHidden:TRUE];
    [self.view addSubview:player_view];
    [self.view bringSubviewToFront:player_view];
}

- (void)viewDidAppear:(BOOL)animated{
    [super viewDidAppear:animated];
    [UIApplication sharedApplication].idleTimerDisabled = YES;
}

- (void)viewDidDisappear:(BOOL)animated{
    [super viewDidDisappear:animated];
    [UIApplication sharedApplication].idleTimerDisabled = NO;
}

#pragma mark -- Setter Getter
- (UIButton*)mCaptureButton{
    if(!_mCaptureButton){
        _mCaptureButton = [UIButton new];
        _mCaptureButton.size = CGSizeMake(60, 30);
        _mCaptureButton.left = 10;
        _mCaptureButton.top = 30;
        [_mCaptureButton setTitle:@"开播" forState:UIControlStateNormal];
        _mCaptureButton.accessibilityIdentifier = @"buttonKaiBo";
        @weakify(self)
        [_mCaptureButton addBlockForControlEvents:UIControlEventTouchUpInside block:^(id  _Nonnull sender) {
            @strongify(self)
            [self.view.window endEditing:YES];
            self.mPlayerSettingView.left = self.view.width;
            [UIView animateWithDuration:0.3 animations:^{
                if(self.mCaptureSettingView.left == 0) self.mCaptureSettingView.left = self.view.width * -1;
                else self.mCaptureSettingView.left = 0;
            }];
            [self.mCapturer Stop];
            [self.mCaptureButton setTitle:@"开播" forState:UIControlStateNormal];
        }];
        [_mCaptureButton setBackgroundColor:[UIColor greenColor]];
    }
    return _mCaptureButton;
}

- (UIButton*)mPlayerButton{
    if(!_mPlayerButton){
        _mPlayerButton = [UIButton new];
        _mPlayerButton.size = CGSizeMake(60, 30);
        _mPlayerButton.right = self.view.width - 10;
        _mPlayerButton.top = 30;
        [_mPlayerButton setTitle:@"播放" forState:UIControlStateNormal];
        _mPlayerButton.accessibilityIdentifier = @"buttonBoFang";
        @weakify(self)
        [_mPlayerButton addBlockForControlEvents:UIControlEventTouchUpInside block:^(id  _Nonnull sender) {
            @strongify(self)
            [self.view.window endEditing:YES];
            self.mCaptureSettingView.left = self.view.width*-1;
            [UIView animateWithDuration:0.3 animations:^{
                if(self.mPlayerSettingView.left == 0) self.mPlayerSettingView.left = self.view.width * 1;
                else self.mPlayerSettingView.left = 0;
            }];

            [self.mPlayer stopPlay];
            
            [self.mPlayerButton setTitle:@"播放" forState:UIControlStateNormal];
        }];
        [_mPlayerButton setBackgroundColor:[UIColor greenColor]];
    }
    return _mPlayerButton;
}

- (UIButton*)cameraButton{
    if(!_cameraButton){
        _cameraButton = [UIButton new];
        _cameraButton.size = CGSizeMake(44, 44);
        _cameraButton.left = self.mCaptureButton.right + 40;
        _cameraButton.top = 30;
        [_cameraButton setImage:[UIImage imageNamed:@"camra_preview"] forState:UIControlStateNormal];
    }
    return _cameraButton;
}

- (UIButton*)logButton{
    if(!_logButton){
        _logButton = [UIButton new];
        _logButton.size = CGSizeMake(44, 44);
        _logButton.right = self.mPlayerButton.left - 40;
        _logButton.top = 30;
        [_logButton setTitle:@"Log" forState:UIControlStateNormal];
    }
    return _logButton;
}

- (LFCaptureSettingView*)mCaptureSettingView {
    if(!_mCaptureSettingView){
        _mCaptureSettingView = [[LFCaptureSettingView alloc] initWithFrame:self.view.bounds];
        _mCaptureSettingView.left = self.view.width*-1;
        _mCaptureSettingView.backgroundColor = [UIColor lightGrayColor];

        @weakify(self)
        _mCaptureSettingView.rtpStartBlock = ^(NSString *appid , NSString *alias, NSString *url, int logLevel){
            @strongify(self)
            [UIView animateWithDuration:0.3 animations:^{
                self.mCaptureSettingView.left = self.view.width * -1;
            }];
            
            [self.mCapturer startCapture:nil];
            [self.mCapturer StartPreview];
            [[self.mCapturer renderView] setHidden:FALSE];
            
            rtcOcNetworkConfig *net = [[rtcOcNetworkConfig alloc] init];
            net.lapi = url;
            net.appid = appid;
            net.alias = alias;
            net.token = @"98765";
            [self.mCapturer StartEncode:nil AndSend:net];

            [self.mCaptureButton setTitle:@"停止" forState:UIControlStateNormal];
        };
    }
    return _mCaptureSettingView;
}

- (LFPlayerSettingView*)mPlayerSettingView {
    if(!_mPlayerSettingView){
        _mPlayerSettingView = [[LFPlayerSettingView alloc] initWithFrame:self.view.bounds];
        _mPlayerSettingView.left = self.view.width*1;
        _mPlayerSettingView.backgroundColor = [UIColor lightGrayColor];
        @weakify(self)
        _mPlayerSettingView.startRtpBlock = ^(NSString *appid, NSString *alias, NSString *hosturl,int logLevel){
            @strongify(self)
            [UIView animateWithDuration:0.3 animations:^{
                self.mPlayerSettingView.left = self.view.width * 1;
            }];
            
            rtcOcNetworkConfig *net = [[rtcOcNetworkConfig alloc] init];
            net.appid = appid;
            net.alias = alias;
            net.lapi = hosturl;
            net.token = @"98765";
            [self.mPlayer startPlay:net];
            [[self.mPlayer renderView] setHidden:TRUE];
            
            [self.mPlayerButton setTitle:@"停止" forState:UIControlStateNormal];
        };
    }
    return _mPlayerSettingView;
}

#pragma mark -- player Notify
-(void)onPlayerVideoWidth:(int)w AndHeight:(int)h {
    @weakify(self)
    dispatch_async(dispatch_get_main_queue(), ^{
        @strongify(self)
        if (self) {
            UIView *player_view = [self.mPlayer renderView];
            player_view.size = CGSizeMake(w * 2/5, h *2/5);
            player_view.right = player_view.right;
            player_view.bottom = player_view.bottom;
        }
    });
}

-(void)onPlayerFirstPicture {
    @weakify(self)
    dispatch_async(dispatch_get_main_queue(), ^{
        @strongify(self)
        if (self) {
            [[self.mPlayer renderView] setHidden:FALSE];
        }
    });
}

-(void)onPlayerStateError {
    @weakify(self)
    dispatch_async(dispatch_get_main_queue(), ^{
        @strongify(self)
        if (self) {
            [self.mPlayer stopPlay];
            UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"播放失败" message:@"播放失败" delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
            [alertView show];
        }
    });
}

@end
