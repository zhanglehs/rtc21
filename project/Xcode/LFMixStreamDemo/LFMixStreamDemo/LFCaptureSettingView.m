//
//  LFSessionTestView.m
//  LFMixStreamDemo
//
//  Created by admin on 16/9/13.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "LFCaptureSettingView.h"
#import <YYKit/YYKit.h>

@interface LFCaptureSettingView ()<UITextFieldDelegate,UIActionSheetDelegate>

@property (nonatomic, strong) UITextField *appIdTextFiled;
@property (nonatomic, strong) UITextField *aliasTextFiled;
@property (nonatomic, strong) UITextField *urlTextFiled;
@property (nonatomic, strong) UITextField *logTextFiled;
@property (nonatomic, strong) UIButton *startPlayButton;
@property (nonatomic, strong) UITextField *bpsTextFiled;
@end

@implementation LFCaptureSettingView

- (instancetype)initWithFrame:(CGRect)frame{
    if(self = [super initWithFrame:frame]){
        [self addSubview:self.urlTextFiled];
        [self addSubview:self.appIdTextFiled];
        [self addSubview:self.aliasTextFiled];
        [self addSubview:self.bpsTextFiled];
        [self addSubview:self.logTextFiled];
        [self addSubview:self.startPlayButton];
        [self setContentSize:CGSizeMake(self.width, self.startPlayButton.bottom)];
        
        @weakify(self)
        UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc] initWithActionBlock:^(id  _Nonnull sender) {
            @strongify(self)
            [self endEditing:YES];
        }];
        [self addGestureRecognizer:tapGesture];
    }
    return self;
}

#pragma mark -- Setter Getter

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
        _aliasTextFiled.text = @"";
        
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
        _bpsTextFiled.text = @"800";
        
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

- (UIButton*)startPlayButton{
    if(!_startPlayButton){
        _startPlayButton = [UIButton new];
        _startPlayButton.size = CGSizeMake(300, 40);
        _startPlayButton.top = self.logTextFiled.bottom + 10;
        _startPlayButton.centerX = self.centerX;
        _startPlayButton.backgroundColor = [UIColor whiteColor];
        [_startPlayButton setTitle:@"开播（普通模式）" forState:UIControlStateNormal];
        _startPlayButton.accessibilityIdentifier = @"ButtonKaiBoNormal";
        [_startPlayButton setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
        @weakify(self)
        [_startPlayButton addBlockForControlEvents:UIControlEventTouchUpInside block:^(id  _Nonnull sender) {
            @strongify(self);
            if(self.appIdTextFiled.text.length > 0 && self.aliasTextFiled.text.length > 0 && self.urlTextFiled.text.length > 0){
                NSString *val = self.aliasTextFiled.text;
                self.aliasTextFiled.text = [val stringByReplacingOccurrencesOfString:@" " withString:@""];
                val = self.appIdTextFiled.text;
                self.appIdTextFiled.text = [val stringByReplacingOccurrencesOfString:@" " withString:@""];
                if (self.rtpStartBlock){
                    int loglevel = 0;
                    if ([self.logTextFiled.text isEqualToString:@"CLIENT-NON"] == YES)
                        loglevel = 1;
                    else if ([self.logTextFiled.text isEqualToString:@"CLIENT-RTP"] == YES)
                        loglevel = 2;
                    self.rtpStartBlock(self.appIdTextFiled.text,self.aliasTextFiled.text,self.urlTextFiled.text,loglevel);
                }
            } else {
                UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"请输入相关参数" message:nil delegate:nil cancelButtonTitle:@"确定" otherButtonTitles:nil, nil];
                [alertView show];
                return ;
            }
        }];
    }
    return _startPlayButton;
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

#pragma mark -- UITextFiledDelegate
- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    [self endEditing:YES];
    return YES;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField {
    self.contentInset = UIEdgeInsetsMake(0, 0, self.contentSize.height - 300, 0);
}

- (void)textFieldDidEndEditing:(UITextField *)textField {
    self.contentInset = UIEdgeInsetsMake(0, 0, 0, 0);
}

#pragma mark UIActionSheetDelegate
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if(actionSheet.tag == 1)
        _urlTextFiled.text = [actionSheet buttonTitleAtIndex:buttonIndex];
    if(actionSheet.tag == 2)
        _logTextFiled.text = [actionSheet buttonTitleAtIndex:buttonIndex];
}

@end
