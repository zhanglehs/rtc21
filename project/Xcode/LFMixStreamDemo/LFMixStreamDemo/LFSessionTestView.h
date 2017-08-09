//
//  LFSessionTestView.h
//  LFMixStreamDemo
//
//  Created by admin on 16/9/13.
//  Copyright © 2016年 admin. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface LFSessionTestView : UIScrollView

@property (nonatomic, copy) void(^flvStartBlock)(NSString *streamId , NSString *ip, NSString *port,NSString *playerStreamId);
@property (nonatomic, copy) void(^rtpStartBlock)(NSString *appid , NSString *alias, NSString *url, int logLevel);
@property (nonatomic, copy) BOOL (^rtpInitBlock)(NSString *appid , NSString *alias, NSString *url, int logLevel);
@property (nonatomic, copy) void(^rtpStartBlockWithExtraParams)(NSString *uploadIp, NSString *uploadUdpPort,NSString *uploadTcpPort,NSString *uploadHttpPort,NSString *uploadStreamId,NSString *mtusize,NSString *enablefec,NSString *enablenack);
@property (nonatomic, assign) BOOL isFLV;
@property (nonatomic, copy, readonly) NSString *appid;
@property (nonatomic, copy, readonly) NSString *alias;
@property (nonatomic, copy, readonly) NSString *host;
@property (nonatomic,copy,readonly) NSString *room;
@property (nonatomic, readwrite) NSMutableArray *roomsArray;
@property (nonatomic, copy) NSString * (^getUploadIp)(void);
@property (nonatomic, copy) int (^getUploadUdpPort)(void);
@property (nonatomic, copy) int (^getUploadTcpPort)(void);
@property (nonatomic, copy) int (^getUploadHttpPort)(void);
@property (nonatomic, copy) NSString * (^getUploadStreamId)(void);
- (NSInteger)bps;
- (void)destroyStream:(NSString*)appid alias:(NSString*)alias urlHost:(NSString*)urlHost;
@end
