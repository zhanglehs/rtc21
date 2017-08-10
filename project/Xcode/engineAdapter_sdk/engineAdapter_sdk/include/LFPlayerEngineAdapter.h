//
//  LFRTPPlayerCore.h
//  LFRTPPlayerCore
//
//  Created by admin on 2016/12/29.
//  Copyright © 2016年 admin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface rtcOcNetworkConfig : NSObject
@property (nonatomic, copy) NSString * _Nonnull lapi;
@property (nonatomic, copy) NSString * _Nonnull appid;
@property (nonatomic, copy) NSString * _Nonnull alias;
@property (nonatomic, copy) NSString * _Nonnull token;
@end

@protocol rtcPlayerEventDelegate <NSObject>
@optional
-(void)onPlayerVideoWidth:(int)w AndHeight:(int)h;
-(void)onPlayerFirstPicture;
-(void)onPlayerStateInitializing;
-(void)onPlayerStatePlaying;
-(void)onPlayerStateRetry;
-(void)onPlayerStateError;
@end

@interface rtcPlayerAdapter : NSObject
-(int)startPlay:(rtcOcNetworkConfig* _Nullable)net;
-(int)stopPlay;
-(int)pausePlay;
-(int)resumePlay;
-(int)setEventDelegate:(id<rtcPlayerEventDelegate> _Nullable)delegate;

-(int)snapShot:(NSString* _Nonnull)var1;
-(int)setMuted:(BOOL)mute;
-(nullable UIView*)renderView;

-(BOOL)isPlaying;
-(int)getVideoWidth;
-(int)getVideoHeight;
@end

/////////////////////////////////////////////////////

@interface rtcOcCaptureConfig : NSObject
@property (nonatomic, copy) NSString * _Nonnull audio_deviceid;
@property (nonatomic, assign) int audio_frequence;
@property (nonatomic, copy) NSString * _Nonnull video_deviceid;
@property (nonatomic, assign) int video_capture_width;
@property (nonatomic, assign) int video_capture_height;
@end

@interface rtcOcEncodeConfig : NSObject
@property (nonatomic, assign) int video_encode_width;
@property (nonatomic, assign) int video_encode_height;
@property (nonatomic, assign) int video_max_fps;
@property (nonatomic, assign) int video_gop;
@property (nonatomic, assign) int video_mtu_size;
@property (nonatomic, assign) int video_bitrate;
@end

@interface rtcCaptureAdapter : NSObject
-(int)startCapture:(rtcOcCaptureConfig* _Nullable)config;
-(int)StartPreview;
-(int)StartEncode:(rtcOcEncodeConfig* _Nullable)encode AndSend:(rtcOcNetworkConfig* _Nonnull)net;
-(nullable UIView*)renderView;
-(void)StopEncodeAndSend;
-(void)StopPreview;
-(void)Stop;
@end

