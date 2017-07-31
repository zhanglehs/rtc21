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
-(int)init:( LFRtpPlayerParam *)item;
-(int)uninit;
-(int)startPlay;
-(int)stopPlay;
-(int)pausePlay;
-(int)resumePlay;
-(int)setPlayerEventListenerDelegate:(id< LFRtpPlayerEventDelegate > _Nullable)delegate;

-(int)snapShot:(NSString * _Nonnull)var1;
-(int)setMuted:(BOOL)mute;
-(nullable UIView*)renderView;

-(BOOL)isPlaying;
-(int)getVideoWidth;
-(int)getVideoHeight;
    
@property (nonatomic, weak) id<LFRtpPlayerEventDelegate> _Nullable mDelegate;
@end

