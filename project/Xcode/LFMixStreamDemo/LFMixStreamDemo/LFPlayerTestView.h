//
//  LFPlayerTestView.h
//  LFMixStreamDemo
//
//  Created by admin on 16/9/13.
//  Copyright © 2016年 admin. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface LFPlayerTestView : UIScrollView

@property (nonatomic, copy) void(^startRtpBlock)(NSString *appid,NSString*alias,NSString*hosturl,int logLevel);
@property (nonatomic, copy) NSString *appid;
@property (nonatomic, copy) NSString *alias;
@property (nonatomic, assign) int logLevel;
@property (nonatomic, copy, readonly) NSString *url;
@property (nonatomic, copy, readonly) NSString *hosturl;
#ifdef MULTIPLAYER
@property (nonatomic, copy) NSString *alias2;
#endif
@end
