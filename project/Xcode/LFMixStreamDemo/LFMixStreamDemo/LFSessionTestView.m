//
//  LFSessionTestView.m
//  LFMixStreamDemo
//
//  Created by admin on 16/9/13.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "LFSessionTestView.h"
#import <YYKit/YYKit.h>

typedef void (^ LFSessionRequestComplete)(_Nullable id info,NSError *_Nullable errorMsg);

@interface LFSessionTestView ()<UITextFieldDelegate,UIActionSheetDelegate>

@property (nonatomic, strong) UITextField *appIdTextFiled;
@property (nonatomic, strong) UITextField *aliasTextFiled;
@property (nonatomic, strong) UITextField *urlTextFiled;
@property (nonatomic, strong) UITextField *logTextFiled;
@property (nonatomic, strong) UITextField *streamIdTextFiled;
@property (nonatomic, strong) UITextField *longStreamIdTextFiled;
@property (nonatomic, strong) UITextField *uploadIpTextFiled;
@property (nonatomic, strong) UITextField *uploadUdpPortTextFiled;
@property (nonatomic, strong) UITextField *uploadTcpPortTextFiled;
@property (nonatomic, strong) UITextField *uploadHttpPortTextFiled;
@property (nonatomic, strong) UITextField *uploadStreamIdTextFiled;
@property (nonatomic, strong) UITextField *mtusizeTextFiled;
@property (nonatomic, strong) UITextField *enablefecTextFiled;
@property (nonatomic, strong) UITextField *enablenackTextFiled;
@property (nonatomic, strong) UIButton *startFLVButton;
@property (nonatomic, strong) UIButton *createFVLSTreamButton;
@property (nonatomic, strong) UIButton *createRTPSTreamButton;
@property (nonatomic, strong) UIButton *createRTPSTreamAndPlayButton;
@property (nonatomic, strong) UIButton *startRTPButton;
@property (nonatomic, copy) NSString *playerStreamId;
@property (nonatomic, assign) BOOL isInit;
@property (nonatomic, strong) UITextField *bpsTextFiled;

@property (nonatomic, strong) NSTimer *mTimer;
@end

@implementation LFSessionTestView

- (instancetype)initWithFrame:(CGRect)frame{
    if(self = [super initWithFrame:frame]){
        [self initial];
    }
    return self;
}

- (void)initial{
    [self addSubview:self.urlTextFiled];
    [self addSubview:self.appIdTextFiled];
//    [self addSubview:self.roomTextFiled];
    [self addSubview:self.aliasTextFiled];
    [self addSubview:self.bpsTextFiled];
    [self addSubview:self.logTextFiled];
    //[self addSubview:self.streamIdTextFiled];
    //[self addSubview:self.longStreamIdTextFiled];
    [self addSubview:self.uploadIpTextFiled];
    [self addSubview:self.uploadUdpPortTextFiled];
    [self addSubview:self.uploadTcpPortTextFiled];
    [self addSubview:self.uploadHttpPortTextFiled];
    [self addSubview:self.uploadStreamIdTextFiled];
    [self addSubview:self.mtusizeTextFiled];
    [self addSubview:self.enablefecTextFiled];
    [self addSubview:self.enablenackTextFiled];
    //[self addSubview:self.createFVLSTreamButton];
    [self addSubview:self.createRTPSTreamAndPlayButton];
    [self addSubview:self.createRTPSTreamButton];
    //[self addSubview:self.startFLVButton];
    [self addSubview:self.startRTPButton];
    
    [self setContentSize:CGSizeMake(self.width, self.startRTPButton.bottom)];
    self.isInit = NO;

    @weakify(self)
    UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc] initWithActionBlock:^(id  _Nonnull sender) {
        @strongify(self)
        [self endEditing:YES];
    }];
    [self addGestureRecognizer:tapGesture];
}

#pragma mark -- Setter Getter
- (NSString*)appid{
    return self.appIdTextFiled.text;
}

- (NSString*)alias{
    return self.aliasTextFiled.text;
}

- (NSString*)host{
    return self.urlTextFiled.text;
}

- (NSInteger)bps{
    return [self.bpsTextFiled.text integerValue];
}

- (UITextField*)urlTextFiled{
    if(!_urlTextFiled){
        _urlTextFiled = [UITextField new];
        _urlTextFiled.size = CGSizeMake(300, 40);
        _urlTextFiled.top = 60;
        _urlTextFiled.centerX = kScreenWidth/2;;
        _urlTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _urlTextFiled.placeholder = @"请输入url前缀";
        _urlTextFiled.backgroundColor = [UIColor whiteColor];
        _urlTextFiled.returnKeyType = UIReturnKeyDone;
        _urlTextFiled.delegate = self;
        //_urlTextFiled.text = @"v.laifeng.com/join_demo";
        _urlTextFiled.text = @"101.201.57.242";
        
        _urlTextFiled.rightViewMode = UITextFieldViewModeAlways;
        _urlTextFiled.rightView = ({
            UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
            button.backgroundColor = [UIColor darkGrayColor];
            button.frame = CGRectMake(0, 0, 40, 40);
            [button setTitle:@"host" forState:UIControlStateNormal];
            [button addTarget:self action:@selector(urlTextTap) forControlEvents:UIControlEventTouchUpInside];
            button;
        });
    }
    return _urlTextFiled;
}

- (UITextField*)appIdTextFiled{
    if(!_appIdTextFiled){
        _appIdTextFiled = [UITextField new];
        _appIdTextFiled.size = CGSizeMake(300, 40);
        _appIdTextFiled.top = self.urlTextFiled.bottom + 10;
        _appIdTextFiled.centerX = kScreenWidth/2;;
        _appIdTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _appIdTextFiled.placeholder = @"请输入appId";
        _appIdTextFiled.backgroundColor = [UIColor whiteColor];
        _appIdTextFiled.returnKeyType = UIReturnKeyDone;
        _appIdTextFiled.delegate = self;
        _appIdTextFiled.text = @"301";
    }

    return _appIdTextFiled;
}

- (UITextField*)aliasTextFiled{
    if(!_aliasTextFiled){
        _aliasTextFiled = [UITextField new];
        _aliasTextFiled.size = CGSizeMake(300, 40);
        _aliasTextFiled.top = self.appIdTextFiled.bottom + 10;
        _aliasTextFiled.centerX = kScreenWidth/2;
        _aliasTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _aliasTextFiled.placeholder = @"请输入alias";
        _aliasTextFiled.backgroundColor = [UIColor whiteColor];
        _aliasTextFiled.returnKeyType = UIReturnKeyDone;
        _aliasTextFiled.delegate = self;
        _aliasTextFiled.text = @"";//stream_alias_380587421_98154978
        
        NSUserDefaults *mNsUserDefaults = [NSUserDefaults standardUserDefaults];
        NSString *string = [mNsUserDefaults objectForKey:@"livealias"];
        if([string length] >0)
            _aliasTextFiled.text = string;
        
    }
    return _aliasTextFiled;
}

- (UITextField*)bpsTextFiled{
    if(!_bpsTextFiled){
        _bpsTextFiled = [UITextField new];
        _bpsTextFiled.size = CGSizeMake(300, 40);
        _bpsTextFiled.top = self.aliasTextFiled.bottom + 10;
        _bpsTextFiled.centerX = kScreenWidth/2;
        _bpsTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _bpsTextFiled.placeholder = @"请输入码率";
        _bpsTextFiled.backgroundColor = [UIColor whiteColor];
        _bpsTextFiled.returnKeyType = UIReturnKeyDone;
        _bpsTextFiled.delegate = self;
        _bpsTextFiled.text = @"800";//stream_alias_380587421_98154978
        
    }
    return _bpsTextFiled;
}

- (UITextField*)logTextFiled{
    if(!_logTextFiled){
        _logTextFiled = [UITextField new];
        _logTextFiled.size = CGSizeMake(300, 40);
        _logTextFiled.top = self.bpsTextFiled.bottom + 10;
        _logTextFiled.centerX = kScreenWidth/2;;
        _logTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _logTextFiled.placeholder = @"是否开启LOG";
        _logTextFiled.backgroundColor = [UIColor whiteColor];
        _logTextFiled.returnKeyType = UIReturnKeyDone;
        _logTextFiled.delegate = self;
        _logTextFiled.text = @"CLIENT-NON";
        
        _logTextFiled.rightViewMode = UITextFieldViewModeAlways;
        _logTextFiled.rightView = ({
            UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
            button.backgroundColor = [UIColor darkGrayColor];
            button.frame = CGRectMake(0, 0, 40, 40);
            [button setTag:2];
            [button setTitle:@"Log" forState:UIControlStateNormal];
            [button addTarget:self action:@selector(logTextTap) forControlEvents:UIControlEventTouchUpInside];
            button;
        });
    }
    return _logTextFiled;
}

- (UITextField*)streamIdTextFiled{
    if(!_streamIdTextFiled){
        _streamIdTextFiled = [UITextField new];
        _streamIdTextFiled.size = CGSizeMake(300, 40);
        _streamIdTextFiled.top = self.createRTPSTreamButton.bottom + 20;
        _streamIdTextFiled.centerX = kScreenWidth/2;;
        _streamIdTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _streamIdTextFiled.placeholder = @"请输入streamId";
        _streamIdTextFiled.backgroundColor = [UIColor whiteColor];
        _streamIdTextFiled.returnKeyType = UIReturnKeyDone;
        _streamIdTextFiled.delegate = self;
    }
    return _streamIdTextFiled;
}

- (UITextField*)longStreamIdTextFiled{
    if(!_longStreamIdTextFiled){
        _longStreamIdTextFiled = [UITextField new];
        _longStreamIdTextFiled.size = CGSizeMake(300, 40);
        _longStreamIdTextFiled.top = self.streamIdTextFiled.bottom + 20;
        _longStreamIdTextFiled.centerX = kScreenWidth/2;;
        _longStreamIdTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _longStreamIdTextFiled.placeholder = @"请输入long streamId";
        _longStreamIdTextFiled.backgroundColor = [UIColor whiteColor];
        _longStreamIdTextFiled.returnKeyType = UIReturnKeyDone;
        _longStreamIdTextFiled.delegate = self;
    }
    return _longStreamIdTextFiled;
}

- (UITextField*)uploadIpTextFiled{
    if(!_uploadIpTextFiled){
        _uploadIpTextFiled = [UITextField new];
        _uploadIpTextFiled.size = CGSizeMake(300, 40);
        _uploadIpTextFiled.top = self.createRTPSTreamButton.bottom + 10;
        _uploadIpTextFiled.centerX = kScreenWidth/2;;
        _uploadIpTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _uploadIpTextFiled.placeholder = @"请输入上传ip";
        _uploadIpTextFiled.backgroundColor = [UIColor whiteColor];
        _uploadIpTextFiled.returnKeyType = UIReturnKeyDone;
        _uploadIpTextFiled.delegate = self;
        //_uploadIpTextFiled.text = @"101.227.10.50";
        //_uploadIpTextFiled.text = @"103.41.143.103";
    }
    return _uploadIpTextFiled;
}

- (UITextField*)uploadUdpPortTextFiled{
    if(!_uploadUdpPortTextFiled){
        _uploadUdpPortTextFiled = [UITextField new];
        _uploadUdpPortTextFiled.size = CGSizeMake(300, 40);
        _uploadUdpPortTextFiled.top = self.uploadIpTextFiled.bottom + 10;
        _uploadUdpPortTextFiled.centerX = kScreenWidth/2;;
        _uploadUdpPortTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _uploadUdpPortTextFiled.placeholder = @"请输入上传udp port";
        _uploadUdpPortTextFiled.backgroundColor = [UIColor whiteColor];
        _uploadUdpPortTextFiled.returnKeyType = UIReturnKeyDone;
        _uploadUdpPortTextFiled.delegate = self;
        //_uploadUdpPortTextFiled.text = @"80";
    }
    return _uploadUdpPortTextFiled;
}

- (UITextField*)uploadTcpPortTextFiled{
    if(!_uploadTcpPortTextFiled){
        _uploadTcpPortTextFiled = [UITextField new];
        _uploadTcpPortTextFiled.size = CGSizeMake(300, 40);
        _uploadTcpPortTextFiled.top = self.uploadUdpPortTextFiled.bottom + 10;
        _uploadTcpPortTextFiled.centerX = kScreenWidth/2;;
        _uploadTcpPortTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _uploadTcpPortTextFiled.placeholder = @"请输入上传tcp port";
        _uploadTcpPortTextFiled.backgroundColor = [UIColor whiteColor];
        _uploadTcpPortTextFiled.returnKeyType = UIReturnKeyDone;
        _uploadTcpPortTextFiled.delegate = self;
        //_uploadTcpPortTextFiled.text = @"80";
    }
    return _uploadTcpPortTextFiled;
}

- (UITextField*)uploadHttpPortTextFiled{
    if(!_uploadHttpPortTextFiled){
        _uploadHttpPortTextFiled = [UITextField new];
        _uploadHttpPortTextFiled.size = CGSizeMake(300, 40);
        _uploadHttpPortTextFiled.top = self.uploadTcpPortTextFiled.bottom + 10;
        _uploadHttpPortTextFiled.centerX = kScreenWidth/2;;
        _uploadHttpPortTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _uploadHttpPortTextFiled.placeholder = @"请输入上传port";
        _uploadHttpPortTextFiled.backgroundColor = [UIColor whiteColor];
        _uploadHttpPortTextFiled.returnKeyType = UIReturnKeyDone;
        _uploadHttpPortTextFiled.delegate = self;
        //_uploadHttpPortTextFiled.text = @"81";
    }
    return _uploadHttpPortTextFiled;
}

- (UITextField*)mtusizeTextFiled{
    if(!_mtusizeTextFiled){
        _mtusizeTextFiled = [UITextField new];
        _mtusizeTextFiled.size = CGSizeMake(300, 40);
        _mtusizeTextFiled.top = self.uploadHttpPortTextFiled.bottom + 10;
        _mtusizeTextFiled.centerX = kScreenWidth/2;;
        _mtusizeTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _mtusizeTextFiled.placeholder = @"请输入mtu size";
        _mtusizeTextFiled.backgroundColor = [UIColor whiteColor];
        _mtusizeTextFiled.returnKeyType = UIReturnKeyDone;
        _mtusizeTextFiled.delegate = self;
        _mtusizeTextFiled.text = @"1200";
    }
    return _mtusizeTextFiled;
}
- (UITextField*)enablefecTextFiled{
    if(!_enablefecTextFiled){
        _enablefecTextFiled = [UITextField new];
        _enablefecTextFiled.size = CGSizeMake(300, 40);
        _enablefecTextFiled.top = self.mtusizeTextFiled.bottom + 10;
        _enablefecTextFiled.centerX = kScreenWidth/2;;
        _enablefecTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _enablefecTextFiled.placeholder = @"EnableFec";
        _enablefecTextFiled.backgroundColor = [UIColor whiteColor];
        _enablefecTextFiled.returnKeyType = UIReturnKeyDone;
        _enablefecTextFiled.delegate = self;
        _enablefecTextFiled.text = @"1";
    }
    return _enablefecTextFiled;
}

- (UITextField*)enablenackTextFiled{
    if(!_enablenackTextFiled){
        _enablenackTextFiled = [UITextField new];
        _enablenackTextFiled.size = CGSizeMake(300, 40);
        _enablenackTextFiled.top = self.enablefecTextFiled.bottom + 10;
        _enablenackTextFiled.centerX = kScreenWidth/2;;
        _enablenackTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _enablenackTextFiled.placeholder = @"EnableNack";
        _enablenackTextFiled.backgroundColor = [UIColor whiteColor];
        _enablenackTextFiled.returnKeyType = UIReturnKeyDone;
        _enablenackTextFiled.delegate = self;
        _enablenackTextFiled.text = @"1";
    }
    return _enablenackTextFiled;
}

- (UITextField*)uploadStreamIdTextFiled{
    if(!_uploadStreamIdTextFiled){
        _uploadStreamIdTextFiled = [UITextField new];
        _uploadStreamIdTextFiled.size = CGSizeMake(300, 40);
        _uploadStreamIdTextFiled.top = self.enablenackTextFiled.bottom + 10;
        _uploadStreamIdTextFiled.centerX = kScreenWidth/2;;
        _uploadStreamIdTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _uploadStreamIdTextFiled.placeholder = @"请输入上传StreamId";
        _uploadStreamIdTextFiled.backgroundColor = [UIColor whiteColor];
        _uploadStreamIdTextFiled.returnKeyType = UIReturnKeyDone;
        _uploadStreamIdTextFiled.delegate = self;
        _uploadStreamIdTextFiled.text = @"";
    }
    return _uploadStreamIdTextFiled;
}

- (UIButton*)startFLVButton{
    if(!_startFLVButton){
        _startFLVButton = [UIButton new];
        _startFLVButton.size = CGSizeMake(300, 40);
        _startFLVButton.top = self.mtusizeTextFiled.bottom + 20;
        _startFLVButton.centerX = self.centerX;
        _startFLVButton.backgroundColor = [UIColor whiteColor];
        [_startFLVButton setTitle:@"开始FLV直播" forState:UIControlStateNormal];
        [_startFLVButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        @weakify(self)
        [_startFLVButton addBlockForControlEvents:UIControlEventTouchUpInside block:^(id  _Nonnull sender) {
            @strongify(self)
            [self endEditing:YES];
            if(!self.isFLV){
                UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"您执行的是Rtp程序,点击Rtp开始开播按钮" message:nil delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
                [alertView show];
                return ;
            }
            if(self.streamIdTextFiled.text.length > 0 && self.uploadUdpPortTextFiled.text.length > 0 && self.uploadIpTextFiled.text.length > 0){
                if(self.flvStartBlock){
                    self.flvStartBlock(self.streamIdTextFiled.text,self.uploadIpTextFiled.text,self.uploadUdpPortTextFiled.text,self.playerStreamId);
                }
            }
        }];
    }
    return _startFLVButton;
}

- (UIButton*)startRTPButton{
    if(!_startRTPButton){
        _startRTPButton = [UIButton new];
        _startRTPButton.size = CGSizeMake(300, 40);
        _startRTPButton.top = self.uploadStreamIdTextFiled.bottom + 10;
        _startRTPButton.centerX = self.centerX;
        _startRTPButton.backgroundColor = [UIColor whiteColor];
        [_startRTPButton setTitle:@"Live RTP Stream" forState:UIControlStateNormal];
        [_startRTPButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        @weakify(self)
        [_startRTPButton addBlockForControlEvents:UIControlEventTouchUpInside block:^(id  _Nonnull sender) {
            @strongify(self)
            if(self.isFLV){
                UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"您执行的是Flv程序,,点击Flv开始开播按钮" message:nil delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
                [alertView show];
                return ;
            }
            if([self.appIdTextFiled.text length] == 0 || [self.aliasTextFiled.text length] == 0 || [self.urlTextFiled.text length] == 0 || [self.uploadIpTextFiled.text length] == 0 || [self.uploadUdpPortTextFiled.text length] == 0 || [self.uploadTcpPortTextFiled.text length] == 0 || [self.uploadHttpPortTextFiled.text length] == 0 || [self.mtusizeTextFiled.text length] == 0|| [self.enablefecTextFiled.text length] == 0|| [self.enablenackTextFiled.text length] == 0 || [self.uploadStreamIdTextFiled.text length] == 0) {
                UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"请输入相关参数" message:nil delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
                [alertView show];
                return ;
            }
            if(self.isInit == YES) {
                if(self.rtpStartBlockWithExtraParams){
                    self.rtpStartBlockWithExtraParams(self.uploadIpTextFiled.text,self.uploadUdpPortTextFiled.text,self.uploadTcpPortTextFiled.text,self.uploadHttpPortTextFiled.text,self.uploadStreamIdTextFiled.text,self.mtusizeTextFiled.text,self.enablefecTextFiled.text,self.enablenackTextFiled.text);
                }
            } else {
                UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"请先点击Create RTP Stream" message:nil delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
                [alertView show];
            }
        }];
    }
    return _startRTPButton;
}

- (UIButton*)createFVLSTreamButton{
    if(!_createFVLSTreamButton){
        _createFVLSTreamButton = [UIButton new];
        _createFVLSTreamButton.size = CGSizeMake(300, 40);
        _createFVLSTreamButton.top = self.aliasTextFiled.bottom + 20;
        _createFVLSTreamButton.centerX = self.centerX;
        _createFVLSTreamButton.backgroundColor = [UIColor whiteColor];
        [_createFVLSTreamButton setTitle:@"Create FLV Stream" forState:UIControlStateNormal];
        [_createFVLSTreamButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        @weakify(self)
        [_createFVLSTreamButton addBlockForControlEvents:UIControlEventTouchUpInside block:^(id  _Nonnull sender) {
            @strongify(self)
            self.isFLV = YES;
            if(self.appIdTextFiled.text.length > 0 && self.aliasTextFiled.text.length > 0 && self.urlTextFiled.text.length > 0){
                @weakify(self)
                [self createStream:self.appIdTextFiled.text alias:self.aliasTextFiled.text urlHost:self.urlTextFiled.text completeBlock:^(id  _Nullable info, NSError * _Nullable errorMsg) {
                    @strongify(self)
                    if(info){
                        dispatch_async(dispatch_get_main_queue(), ^{
                            self.streamIdTextFiled.text = info;
                            self.longStreamIdTextFiled.text = self.playerStreamId;
                        });
                    }
                }];
            }
        }];
    }
    return _createFVLSTreamButton;
}

- (UIButton*)createRTPSTreamAndPlayButton{
    if(!_createRTPSTreamAndPlayButton){
        _createRTPSTreamAndPlayButton = [UIButton new];
        _createRTPSTreamAndPlayButton.size = CGSizeMake(300, 40);
        _createRTPSTreamAndPlayButton.top = self.logTextFiled.bottom + 10;
        _createRTPSTreamAndPlayButton.centerX = self.centerX;
        _createRTPSTreamAndPlayButton.backgroundColor = [UIColor whiteColor];
        [_createRTPSTreamAndPlayButton setTitle:@"开播（普通模式）" forState:UIControlStateNormal];
        _createRTPSTreamAndPlayButton.accessibilityIdentifier = @"ButtonKaiBoNormal";
        [_createRTPSTreamAndPlayButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        @weakify(self)
        [_createRTPSTreamAndPlayButton addBlockForControlEvents:UIControlEventTouchUpInside block:^(id  _Nonnull sender) {
            @strongify(self);
            self.isFLV = NO;
            if(self.appIdTextFiled.text.length > 0 && self.aliasTextFiled.text.length > 0 && self.urlTextFiled.text.length > 0){
                NSString *val = self.alias;
                self.aliasTextFiled.text = [val stringByReplacingOccurrencesOfString:@" " withString:@""];
                val = self.appid;
                self.appIdTextFiled.text = [val stringByReplacingOccurrencesOfString:@" " withString:@""];
                
                NSUserDefaults *mNsUserDefaults = [NSUserDefaults standardUserDefaults];
                [mNsUserDefaults setValue:self.alias forKey:@"livealias"];
                [mNsUserDefaults synchronize];
                [mNsUserDefaults setValue:self.room forKey:@"roomid"];
                [mNsUserDefaults synchronize];
                [mNsUserDefaults setValue:self.appid forKey:@"appid"];
                [mNsUserDefaults synchronize];
                [self createStream:self.appIdTextFiled.text alias:self.aliasTextFiled.text urlHost:self.urlTextFiled.text completeBlock:^(id  _Nullable info, NSError * _Nullable errorMsg) {
                    if(!errorMsg){
                        dispatch_async(dispatch_get_main_queue(), ^{
                            if(self.rtpStartBlock){
                                int loglevel = 0;
                                if ([self.logTextFiled.text isEqualToString:@"CLIENT-NON"] == YES)
                                    loglevel = 1;
                                else if ([self.logTextFiled.text isEqualToString:@"CLIENT-RTP"] == YES)
                                    loglevel = 2;
                                self.rtpStartBlock(self.appIdTextFiled.text,self.aliasTextFiled.text,self.urlTextFiled.text,loglevel);
                            }
                        });
                    }
                    /*
                    @strongify(self)
                    if(info){
                        dispatch_async(dispatch_get_main_queue(), ^{
                            self.streamIdTextFiled.text = info;
                            self.longStreamIdTextFiled.text = self.playerStreamId;
                        });
                    }
                    */
                }];
            } else {
                UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"请输入相关参数" message:nil delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
                [alertView show];
                return ;
            }
        }];
    }
    return _createRTPSTreamAndPlayButton;
}

- (UIButton*)createRTPSTreamButton{
    if(!_createRTPSTreamButton){
        _createRTPSTreamButton = [UIButton new];
        _createRTPSTreamButton.size = CGSizeMake(300, 40);
        _createRTPSTreamButton.top = self.createRTPSTreamAndPlayButton.bottom + 30;
        _createRTPSTreamButton.centerX = self.centerX;
        _createRTPSTreamButton.backgroundColor = [UIColor whiteColor];
        [_createRTPSTreamButton setTitle:@"Create RTP Stream" forState:UIControlStateNormal];
        [_createRTPSTreamButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        @weakify(self)
        [_createRTPSTreamButton addBlockForControlEvents:UIControlEventTouchUpInside block:^(id  _Nonnull sender) {
            @strongify(self);
            self.isFLV = NO;
            if(self.appIdTextFiled.text.length > 0 && self.aliasTextFiled.text.length > 0 && self.urlTextFiled.text.length > 0){
                NSUserDefaults *mNsUserDefaults = [NSUserDefaults standardUserDefaults];
                [mNsUserDefaults setValue:self.alias forKey:@"livealias"];
                [mNsUserDefaults synchronize];
                [mNsUserDefaults setValue:self.room forKey:@"roomid"];
                [mNsUserDefaults synchronize];
                [mNsUserDefaults setValue:self.appid forKey:@"appid"];
                [mNsUserDefaults synchronize];
                [self createStream:self.appIdTextFiled.text alias:self.aliasTextFiled.text urlHost:self.urlTextFiled.text completeBlock:^(id  _Nullable info, NSError * _Nullable errorMsg) {
                    if(!errorMsg){
                        dispatch_async(dispatch_get_main_queue(), ^{
                            BOOL ret = NO;
                            if(self.rtpInitBlock){
                                int loglevel = 0;
                                if ([self.logTextFiled.text isEqualToString:@"CLIENT-NON"] == YES)
                                    loglevel = 1;
                                else if ([self.logTextFiled.text isEqualToString:@"CLIENT-RTP"] == YES)
                                    loglevel = 2;
                                ret = self.rtpInitBlock(self.appIdTextFiled.text,self.aliasTextFiled.text,self.urlTextFiled.text,loglevel);
                            }
                            if(ret == YES) {
                                if(self.getUploadIp){
                                    self.uploadIpTextFiled.text = @"";
                                }
                                if(self.getUploadUdpPort){
                                    self.uploadUdpPortTextFiled.text = @"";
                                }
                                if(self.getUploadTcpPort){
                                    self.uploadTcpPortTextFiled.text = @"";
                                }
                                if(self.getUploadHttpPort){
                                    self.uploadHttpPortTextFiled.text = @"";
                                }
                                if(self.getUploadStreamId){
                                    self.uploadStreamIdTextFiled.text = @"";
                                }
                                _mTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                                           target:self
                                                                         selector:@selector(tick)
                                                                         userInfo:nil
                                                                          repeats:YES];
                                self.isInit = YES;
                            }
                        });
                    }
                }];
            } else {
                UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"请输入相关参数" message:nil delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
                [alertView show];
                return ;
            }
        }];
    }
    return _createRTPSTreamButton;
}

#pragma mark -- UITextFiledDelegate
- (BOOL)textFieldShouldReturn:(UITextField *)textField{
    [self endEditing:YES];
    return YES;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField{
    self.contentInset = UIEdgeInsetsMake(0, 0, self.contentSize.height - 300, 0);
}

- (void)textFieldDidEndEditing:(UITextField *)textField{
    self.contentInset = UIEdgeInsetsMake(0, 0, 0, 0);
}

#pragma mark -- Request

- (void)createStream:(NSString*)appid alias:(NSString*)alias urlHost:(NSString*)urlHost completeBlock:(LFSessionRequestComplete)completeBlock{
    LFSessionRequestComplete completeBlockCopy = [completeBlock copy];
    
    NSString* url = [NSString stringWithFormat:@"http://%@/v1/create_stream",urlHost];
    
    NSString *pair1 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"app_id"), LFSessionPercentEscapedStringFromString(appid)];
    NSString *pair2 = nil;
    if(self.isFLV){
        pair2 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"stream_type"), LFSessionPercentEscapedStringFromString(@"pc_plugin")];
    }else{
        pair2 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"stream_type"), LFSessionPercentEscapedStringFromString(@"rtp")];
    }

    NSString *pair3 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"res"), LFSessionPercentEscapedStringFromString(@"360x640")];
    NSString *pair4 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"rt"), LFSessionPercentEscapedStringFromString(@"300")];
    NSString *pair5 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"alias"), LFSessionPercentEscapedStringFromString(alias)];
    
    NSString *pair6 = nil;
    if(self.isFLV){
        pair6 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"stream_format"), LFSessionPercentEscapedStringFromString(@"flv")];
    }else{
        pair6 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"stream_format"), LFSessionPercentEscapedStringFromString(@"rtp")];
    }

    NSString *pair7 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"nt"), LFSessionPercentEscapedStringFromString(@"0")];
    NSString *pair8 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"token"), LFSessionPercentEscapedStringFromString(@"98765")];
    NSString *pair9 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"is_open_mix"), LFSessionPercentEscapedStringFromString(@"0")];
    NSString *pair10 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"role"), LFSessionPercentEscapedStringFromString(@"slave")];
    url = [NSString stringWithFormat:@"%@?%@&%@&%@&%@&%@&%@&%@&%@&%@&%@",url,pair1,pair2,pair3,pair4,pair5,pair6,pair7,pair8,pair9,pair10];
    NSLog(@"createstream url:%@", url);
    NSURLSessionConfiguration *configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
    NSURLSession *session = [NSURLSession sessionWithConfiguration:configuration];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:url]];
    request.HTTPMethod = @"GET";
    request.cachePolicy = NSURLRequestUseProtocolCachePolicy;
    request.timeoutInterval = 10;
    
    @weakify(self)
    NSURLSessionDataTask *localDataTask = [session dataTaskWithRequest:request completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        @strongify(self)
        if(error){
            dispatch_async(dispatch_get_main_queue(), ^{
                UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Createsream Error" message:[error localizedDescription] delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
                [alertView show];
            });
            if(completeBlockCopy) completeBlockCopy(nil,error);
        }else{
            NSError *jsonError = nil;
            NSDictionary *jsonObject = [NSJSONSerialization JSONObjectWithData:data options:NSJSONReadingAllowFragments error:&jsonError];
            if(jsonError){
                if(completeBlockCopy) completeBlockCopy(nil,jsonError);
            }else{
                NSDictionary *old = jsonObject[@"old"];
                if(old){
                    NSDictionary *download = old[@"download"];
                    if(download){
                        NSArray *downLoadList = download[@"download_list"];
                        if(downLoadList && downLoadList.count > 0){
                            self.playerStreamId = [[downLoadList firstObject] objectForKey:@"stream_id"];
                        }
                    }
                }

                if(jsonObject[@"stream_id"]){
                    if(completeBlockCopy) completeBlockCopy(jsonObject[@"stream_id"],nil);
                }
                else{
                    dispatch_async(dispatch_get_main_queue(), ^{
                        UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"Createsream Error" message:@"" delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
                        [alertView show];
                    });
                }
            }
        }
        
    }];
    [localDataTask resume];
    [session finishTasksAndInvalidate];
}

- (void)destroyStream:(NSString*)appid alias:(NSString*)alias urlHost:(NSString*)urlHost {
    NSString* url = [NSString stringWithFormat:@"http://%@/v1/destroy_stream",urlHost];
    
    NSString *pair1 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"app_id"), LFSessionPercentEscapedStringFromString(appid)];
    NSString *pair2 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"alias"), LFSessionPercentEscapedStringFromString(alias)];
    NSString *pair3 = [NSString stringWithFormat:@"%@=%@", LFSessionPercentEscapedStringFromString(@"token"), LFSessionPercentEscapedStringFromString(@"98765")];
    url = [NSString stringWithFormat:@"%@?%@&%@&%@",url,pair1,pair2,pair3];
    NSLog(@"destroystream url:%@", url);
    NSURLSessionConfiguration *configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
    NSURLSession *session = [NSURLSession sessionWithConfiguration:configuration];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:url]];
    request.HTTPMethod = @"GET";
    request.cachePolicy = NSURLRequestUseProtocolCachePolicy;
    request.timeoutInterval = 1000;
    
    NSURLSessionDataTask *localDataTask = [session dataTaskWithRequest:request completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        if(error){
        }else{
        }
        
    }];
    [localDataTask resume];
    [session finishTasksAndInvalidate];
}

NSString * LFSessionPercentEscapedStringFromString(NSString *string) {
    static NSString * const kAFCharactersGeneralDelimitersToEncode = @":#[]@"; // does not include "?" or "/" due to RFC 3986 - Section 3.4
    static NSString * const kAFCharactersSubDelimitersToEncode = @"!$&'()*+,;=";
    
    NSMutableCharacterSet * allowedCharacterSet = [[NSCharacterSet URLQueryAllowedCharacterSet] mutableCopy];
    [allowedCharacterSet removeCharactersInString:[kAFCharactersGeneralDelimitersToEncode stringByAppendingString:kAFCharactersSubDelimitersToEncode]];
    
    // FIXME: https://github.com/AFNetworking/AFNetworking/pull/3028
    // return [string stringByAddingPercentEncodingWithAllowedCharacters:allowedCharacterSet];
    
    static NSUInteger const batchSize = 50;
    
    NSUInteger index = 0;
    NSMutableString *escaped = @"".mutableCopy;
    
    while (index < string.length) {
        NSUInteger length = MIN(string.length - index, batchSize);
        NSRange range = NSMakeRange(index, length);
        
        // To avoid breaking up character sequences such as 👴🏻👮🏽
        range = [string rangeOfComposedCharacterSequencesForRange:range];
        
        NSString *substring = [string substringWithRange:range];
        NSString *encoded = [substring stringByAddingPercentEncodingWithAllowedCharacters:allowedCharacterSet];
        [escaped appendString:encoded];
        
        index += range.length;
    }
    
    return escaped;
}

- (void)urlTextTap
{
    UIActionSheet *actionSheet = [[UIActionSheet alloc] initWithTitle:nil delegate:self cancelButtonTitle:nil destructiveButtonTitle:nil otherButtonTitles:@"101.201.57.242",@"101.200.47.145",@"v.laifeng.com/join_demo", nil];
    [actionSheet setTag:1];
    [actionSheet showInView:self];
}

- (void)logTextTap
{
    UIActionSheet *actionSheet = [[UIActionSheet alloc] initWithTitle:nil delegate:self cancelButtonTitle:nil destructiveButtonTitle:nil otherButtonTitles:@"SEVER-LEVEL",@"CLIENT-NON",@"CLIENT-RTP", nil];
    [actionSheet setTag:2];
    [actionSheet showInView:self];
}

#pragma mark UIActionSheetDelegate
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if(actionSheet.tag == 1)
        _urlTextFiled.text = [actionSheet buttonTitleAtIndex:buttonIndex];
    if(actionSheet.tag == 2)
        _logTextFiled.text = [actionSheet buttonTitleAtIndex:buttonIndex];
}

-(void)tick{
    NSString *ip = self.getUploadIp();
    if(ip == nil || ip.length <= 0){
        NSLog(@"ip:%@\n",ip);
        return ;
    }
    if(self.getUploadIp){
        self.uploadIpTextFiled.text = ip;
    }
    if(self.getUploadUdpPort){
        self.uploadUdpPortTextFiled.text = @(self.getUploadUdpPort()).stringValue;
    }
    if(self.getUploadTcpPort){
        self.uploadTcpPortTextFiled.text = @(self.getUploadTcpPort()).stringValue;
    }
    if(self.getUploadHttpPort){
        self.uploadHttpPortTextFiled.text = @(self.getUploadHttpPort()).stringValue;
    }
    if(self.getUploadStreamId){
        self.uploadStreamIdTextFiled.text = self.getUploadStreamId();
    }
    [_mTimer invalidate];
    _mTimer = nil;
}
@end
