//
//  LFPlayerTestView.h
//  LFMixStreamDemo
//
//  Created by admin on 16/9/13.
//  Copyright © 2016年 admin. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface LFPlayerSettingView : UIScrollView

@property (nonatomic, copy) void(^startRtpBlock)(NSString *appid,NSString*alias,NSString*hosturl,int logLevel);

@end
