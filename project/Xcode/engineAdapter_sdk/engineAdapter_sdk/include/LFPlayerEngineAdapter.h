//
//  LFRTPPlayerCore.h
//  LFRTPPlayerCore
//
//  Created by admin on 2016/12/29.
//  Copyright © 2016年 admin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface LFRtpPlayerParam : NSObject
@property (nonatomic, copy) NSString * _Nonnull alias;
@property (nonatomic, copy) NSString * _Nonnull appId;
@property (nonatomic, copy) NSString * _Nonnull token;
@property (nonatomic, assign) CGSize videoSize;
@property (nonatomic, copy) NSString * _Nonnull lapiUrl;
@end

@protocol LFRtpPlayerEventDelegate <NSObject>
-(void)onPlayerError:(int)event msg:(NSString * _Nonnull)msg;
-(void)onPlayerDecodeVideoSize:(int)w height:(int)h;
-(void)onPlayerStartLoading:(int)event msg:(NSString * _Nonnull)msg;
-(void)onPlayerEndLoading:(int)event msg:(NSString * _Nonnull)msg;
-(void)onPlayerFirstPicture:(int)event msg:(NSString * _Nonnull)msg;
@end

@interface LFPlayerEngineAdapter : NSObject
-(int)init:(LFRtpPlayerParam* _Nonnull)item;
-(int)uninit;
-(int)startPlay;
-(int)stopPlay;
-(int)pausePlay;
-(int)resumePlay;
-(int)setPlayerEventListenerDelegate:(id< LFRtpPlayerEventDelegate > _Nullable)delegate;

-(int)snapShot:(NSString* _Nonnull)var1;
-(int)setMuted:(BOOL)mute;
-(nullable UIView*)renderView;

-(BOOL)isPlaying;
-(int)getVideoWidth;
-(int)getVideoHeight;
    
@property (nonatomic, weak) id<LFRtpPlayerEventDelegate> _Nullable mDelegate;
@end




/////////////////////////////////////////////////////

@interface rtcOcCaptureConfig : NSObject
@property (nonatomic, copy) NSString * _Nonnull audio_deviceid;
@property (nonatomic, assign) int audio_frequence;
@property (nonatomic, copy) NSString * _Nonnull video_deviceid;
@property (nonatomic, assign) int video_capture_width;
@property (nonatomic, assign) int video_capture_height;
-(id _Nonnull)init;
@end

@interface rtcOcEncodeConfig : NSObject
@property (nonatomic, assign) int video_encode_width;
@property (nonatomic, assign) int video_encode_height;
@property (nonatomic, assign) int video_max_fps;
@property (nonatomic, assign) int video_gop;
@property (nonatomic, assign) int video_mtu_size;
@property (nonatomic, assign) int video_bitrate;
-(id _Nonnull)init;
@end

@interface rtcOcNetworkConfig : NSObject
@property (nonatomic, copy) NSString * _Nonnull lapi;
@property (nonatomic, copy) NSString * _Nonnull appid;
@property (nonatomic, copy) NSString * _Nonnull alias;
@property (nonatomic, copy) NSString * _Nonnull token;
@end

@interface rtcCaptureAdapter : NSObject
-(id _Nonnull)initWith:(const CGSize* _Nonnull)size;
-(int)startCapture:(rtcOcCaptureConfig* _Nullable)config;
-(int)StartPreview;
-(int)StartEncode:(rtcOcEncodeConfig* _Nullable)encode AndSend:(rtcOcNetworkConfig* _Nonnull)net;
-(nullable UIView*)renderView;
-(void)StopEncodeAndSend;
-(void)StopPreview;
-(void)Stop;
@end

