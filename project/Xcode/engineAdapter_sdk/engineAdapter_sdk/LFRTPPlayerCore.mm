//
//  LFRTPPlayerCore.m
//  LFRTPPlayerCore
//
//  Created by admin on 2016/12/29.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "include/LFRTPPlayerCore.h"
#import "include/LFVideoItem.h"
#import "LFPlayerEngineAdapter.h"

@interface LFRTPPlayerCore() <LFRtpPlayerEventDelegate>
@property (nonatomic, weak) id<LFPlayerDelegate> _Nullable delegate;    ///< 代理
@property (nonatomic,strong) LFPlayerEngineAdapter *mPlayerEngine;
@end

@implementation LFRTPPlayerCore

#pragma mark -- LifeCycle
+ (nullable instancetype)playerWithVideoItem:(LFVideoItem *)item
{
    LFRTPPlayerCore *player = [[LFRTPPlayerCore alloc] initWithVideoItem:item];
    return player;
}

- (nullable instancetype)initWithVideoItem:(LFVideoItem *)item
{
    self = [super init];
    if(self){
        LFRtpPlayerParam *playerParam = [[LFRtpPlayerParam alloc] init];
        playerParam.appId = item.appId;
        playerParam.alias = item.alias;
        playerParam.lapiUrl = item.hosturl;
        playerParam.token = item.token;
        playerParam.videoSize = item.videoSize;
        [self.mPlayerEngine init:playerParam];

    }
    return self;
}

- (void)dealloc
{
    self.delegate = nil;
    self.mPlayerEngine = nil;
}

#pragma mark -- LFPlayerCore protocol
- (void)prepareToPlay
{
    [self.mPlayerEngine startPlay];
}

- (void)pause
{
    [self.mPlayerEngine pausePlay];
}

- (void)resume
{
    [self.mPlayerEngine resumePlay];
}

- (void)stop
{
    [self.mPlayerEngine stopPlay];
}

- (void)setNetworkChanged{
}

- (BOOL)isPlaying
{
    return [self.mPlayerEngine isPlaying];
}

- (BOOL)isSeeking{
    return 0;
}

- (nullable UIView*)renderView
{
    return [self.mPlayerEngine renderView];
}

- (nullable UIImage*)shotImage
{
    @synchronized (self) {
        if([self renderView] && (![self renderView].hidden)){
            return [self snapshotInternalOnIOS7AndLater];
        }
        return nil;
    }
}

- (UIImage*)snapshotInternalOnIOS7AndLater
{
    CGSize size = [UIScreen mainScreen].bounds.size;
    UIGraphicsBeginImageContextWithOptions(size, NO, 0.0);
    // Render our snapshot into the image context
    [[self renderView] drawViewHierarchyInRect:[UIScreen mainScreen].bounds afterScreenUpdates:NO];
    
    // Grab the image from the context
    UIImage *complexViewImage = UIGraphicsGetImageFromCurrentImageContext();
    
    // Finish using the context
    UIGraphicsEndImageContext();
    
    UIImage* flippedImage = [UIImage imageWithCGImage:complexViewImage.CGImage
                                                scale:complexViewImage.scale
                                          orientation:UIImageOrientationDownMirrored];
    
    return flippedImage;
}

- (void)setMuted:(BOOL)mute
{
    [self.mPlayerEngine setMuted:mute];
}

- (void)setDelegate:(id<LFPlayerDelegate> _Nullable)delegate
{
    _delegate = delegate;
}

- (void)setScalingAspectFill:(BOOL)isFill
{
}

- (void)setAutoPauseInBackground:(BOOL)pause
{
}

- (void)setIsLivePlay:(BOOL)isLivePlay
{
}

- (void)seek:(NSTimeInterval)currentPlaybackTime
{
}

- (void)setCallBackPlayTimeInterval:(NSTimeInterval)callBackTimerInterval
{
}

- (NSTimeInterval)duration
{
    return 0;
}

- (NSTimeInterval)playableDuration
{
    return 0;
}

- (NSTimeInterval)playBackTime
{
    return 0;
}

/**
 帧率
 
 @return CGFloat
 */
- (CGFloat)videoFrameRate;
{
    return 1.0;
}

/**
 平均码率
 
 @return int64_t
 */
- (int64_t)avgvideoBitRate
{
    return 1.0;
}

/**
 I帧平均大小
 
 @return int64_t
 */
- (int64_t)avgKeyFrameSize
{
    return 1.0;
}

- (void)setAutoRunloop:(BOOL)autoRunloop
{
    return;
}

-(int)snapShot:(NSString *)var1
{
    return [self.mPlayerEngine snapShot:var1];
}
    
-(LFPlayerEngineAdapter *)mPlayerEngine{
    if(!_mPlayerEngine){
        _mPlayerEngine = [LFPlayerEngineAdapter alloc];
        [_mPlayerEngine setPlayerEventListenerDelegate:self];
    }
    return _mPlayerEngine;
}

-(void)onPlayerError:(int)event msg:(NSString * _Nonnull)msg
{
    if(self.delegate && [self.delegate respondsToSelector:@selector(playerNotify:)]){
        [self.delegate playerNotify:LFPlayerPlayErrorCode];
    }
}
-(void)onPlayerDecodeVideoSize:(int)w height:(int)h
{
    if(self.delegate && [self.delegate respondsToSelector:@selector(playerNaturalSize:)]){
        [self.delegate playerNaturalSize:CGSizeMake(w, h)];
    }
}
-(void)onPlayerStartLoading:(int)event msg:(NSString * _Nonnull)msg
{
    if(self.delegate && [self.delegate respondsToSelector:@selector(playerNotify:)]){
        [self.delegate playerNotify:LFPlayerPlayStartLoadingCode];
    }
}

-(void)onPlayerEndLoading:(int)event msg:(NSString * _Nonnull)msg
{
    if(self.delegate && [self.delegate respondsToSelector:@selector(playerNotify:)]){
        [self.delegate playerNotify:LFPlayerPlayEndLoadingCode];
    }
}
-(void)onPlayerFirstPicture:(int)event msg:(NSString * _Nonnull)msg
{
    if(self.delegate && [self.delegate respondsToSelector:@selector(playerNotify:)]){
        [self.delegate playerNotify:LFPlayerFirstVideoFrameCode];
    }
}
    
@end
