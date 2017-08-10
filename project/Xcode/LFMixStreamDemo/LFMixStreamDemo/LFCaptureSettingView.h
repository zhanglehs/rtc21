//
//  LFSessionTestView.h
//  LFMixStreamDemo
//
//  Created by admin on 16/9/13.
//  Copyright © 2016年 admin. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface LFCaptureSettingView : UIScrollView

@property (nonatomic, copy) void(^rtpStartBlock)(NSString *appid , NSString *alias, NSString *url, int logLevel);

@end
