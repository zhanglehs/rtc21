//
//  LFPlayerTestView.m
//  LFMixStreamDemo
//
//  Created by admin on 16/9/13.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "LFPlayerSettingView.h"
#import <YYKit/YYKit.h>

@interface LFPlayerSettingView ()<UITextViewDelegate,UITextFieldDelegate,UIActionSheetDelegate>

@property (nonatomic, strong) UITextView *urlTextView;
@property (nonatomic, strong) UITextField *appIdTextFiled;
@property (nonatomic, strong) UITextField *aliasTextFiled;
@property (nonatomic, strong) UITextField *logTextFiled;
@property (nonatomic, strong) UITextField *urlTextFiled;
@property (nonatomic, strong) UIButton *startRtpButton;

@end

@implementation LFPlayerSettingView

- (instancetype)initWithFrame:(CGRect)frame{
    if(self = [super initWithFrame:frame]){
        [self addSubview:self.appIdTextFiled];
        [self addSubview:self.aliasTextFiled];
        [self addSubview:self.logTextFiled];
        [self addSubview:self.urlTextFiled];
        [self addSubview:self.startRtpButton];

        @weakify(self)
        UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc] initWithActionBlock:^(id  _Nonnull sender) {
            @strongify(self)
            [self endEditing:YES];
        }];
        [self addGestureRecognizer:tapGesture];
    }
    return self;
}

#pragma mark -- Getter Setter

- (UITextView*)urlTextView{
    if(!_urlTextView){
        _urlTextView = [UITextView new];
        _urlTextView.size = CGSizeMake(300, 50);
        _urlTextView.top = 60;
        _urlTextView.centerX = kScreenWidth/2;
        _urlTextView.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _urlTextView.backgroundColor = [UIColor whiteColor];
        _urlTextView.returnKeyType = UIReturnKeyDone;
        _urlTextView.delegate = self;
    }
    return _urlTextView;
}

- (void)urlTextTap
{
    UIActionSheet *actionSheet = [[UIActionSheet alloc] initWithTitle:nil delegate:self cancelButtonTitle:nil destructiveButtonTitle:nil otherButtonTitles:@"101.201.57.242",@"101.200.47.145",@"v.laifeng.com/join_demo", nil];
    [actionSheet setTag : 1];
    [actionSheet showInView:self];
}

- (void)logTextTap
{
    UIActionSheet *actionSheet = [[UIActionSheet alloc] initWithTitle:nil delegate:self cancelButtonTitle:nil destructiveButtonTitle:nil otherButtonTitles:@"SEVER-LEVEL",@"CLIENT-NON",@"CLIENT-RTP", nil];
    [actionSheet setTag : 2];
    [actionSheet showInView:self];
}

- (void)setAlias:(NSString *)alias{
    _aliasTextFiled.text = alias;
}

- (void)setAppid:(NSString *)appid{
    _appIdTextFiled.text = appid;
}

- (UITextField*)urlTextFiled{
    if(!_urlTextFiled){
        _urlTextFiled = [UITextField new];
        _urlTextFiled.size = CGSizeMake(300, 40);
        _urlTextFiled.top = 60;
        _urlTextFiled.centerX = kScreenWidth/2;;
        _urlTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _urlTextFiled.placeholder = @"请输入appId";
        _urlTextFiled.backgroundColor = [UIColor whiteColor];
        _urlTextFiled.returnKeyType = UIReturnKeyDone;
        _urlTextFiled.delegate = self;
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
        _aliasTextFiled.centerX = kScreenWidth/2;;
        _aliasTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _aliasTextFiled.placeholder = @"请输入alias";
        _aliasTextFiled.backgroundColor = [UIColor whiteColor];
        _aliasTextFiled.returnKeyType = UIReturnKeyDone;
        _aliasTextFiled.delegate = self;
        _aliasTextFiled.text = @"";
        
        NSUserDefaults *mNsUserDefaults = [NSUserDefaults standardUserDefaults];
        NSString *string = [mNsUserDefaults objectForKey:@"playalias"];
        if([string length] >0)
            _aliasTextFiled.text = string;
    }
    return _aliasTextFiled;
}

- (UITextField*)logTextFiled{
    if(!_logTextFiled){
        _logTextFiled = [UITextField new];
        _logTextFiled.size = CGSizeMake(300, 40);
        _logTextFiled.top = self.aliasTextFiled.bottom + 10;;
        _logTextFiled.centerX = kScreenWidth/2;;
        _logTextFiled.autoresizingMask = UIViewAutoresizingFlexibleBottomMargin;
        _logTextFiled.placeholder = @"请输入appId";
        _logTextFiled.backgroundColor = [UIColor whiteColor];
        _logTextFiled.returnKeyType = UIReturnKeyDone;
        _logTextFiled.delegate = self;
        _logTextFiled.text = @"CLIENT-NON";
        
        _logTextFiled.rightViewMode = UITextFieldViewModeAlways;
        _logTextFiled.rightView = ({
            UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
            button.backgroundColor = [UIColor darkGrayColor];
            button.frame = CGRectMake(0, 0, 40, 40);
            [button setTitle:@"log" forState:UIControlStateNormal];
            [button addTarget:self action:@selector(logTextTap) forControlEvents:UIControlEventTouchUpInside];
            button;
        });
    }
    return _logTextFiled;
}

- (UIButton*)startRtpButton{
    if(!_startRtpButton){
        _startRtpButton = [UIButton new];
        _startRtpButton.size = CGSizeMake(300, 40);
        _startRtpButton.top = self.logTextFiled.bottom + 10;
        _startRtpButton.centerX = self.centerX;
        _startRtpButton.backgroundColor = [UIColor whiteColor];
        [_startRtpButton setTitle:@"播放（普通模式）" forState:UIControlStateNormal];
        [_startRtpButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        _startRtpButton.accessibilityIdentifier = @"buttonBoFangNormal";
        
        @weakify(self)
        [_startRtpButton addBlockForControlEvents:UIControlEventTouchUpInside block:^(id  _Nonnull sender) {
            @strongify(self)
            [self endEditing:YES];
            NSUserDefaults *mNsUserDefaults = [NSUserDefaults standardUserDefaults];
            [mNsUserDefaults setValue:self.aliasTextFiled.text forKey:@"playalias"];
            [mNsUserDefaults synchronize];
            int loglevel = 0;
            if ([self.logTextFiled.text isEqualToString:@"CLIENT-NON"] == YES)
                loglevel = 1;
            else if ([self.logTextFiled.text isEqualToString:@"CLIENT-RTP"] == YES)
                loglevel = 2;
            if(self.startRtpBlock) {
                self.startRtpBlock(self.appIdTextFiled.text, self.aliasTextFiled.text, self.urlTextFiled.text, loglevel);
            }
        }];
    }
    return _startRtpButton;
}

#pragma mark -- UITextFiledDelegate
- (BOOL)textFieldShouldReturn:(UITextField *)textField{
    [self endEditing:YES];
    return YES;
}

#pragma mark UIActionSheetDelegate
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if(actionSheet.tag == 1)
        _urlTextFiled.text = [actionSheet buttonTitleAtIndex:buttonIndex];
    else if(actionSheet.tag == 2)
        _logTextFiled.text = [actionSheet buttonTitleAtIndex:buttonIndex];
}
@end
