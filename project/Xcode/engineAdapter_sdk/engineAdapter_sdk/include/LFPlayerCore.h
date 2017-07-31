//
//  LFPlayerProtocol.h
//  Pods
//
//  Created by admin on 2017/2/17.
//
//

#ifndef LFPlayerProtocol_h
#define LFPlayerProtocol_h
#import "LFVideoItem.h"
#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

///< 首帧渲染时间长度 单位:ms
#define LFMPMoviePlayerFirstVideoFrameRenderedNotification @"LFMPMoviePlayerFirstVideoFrameRendered"

static const NSString *KLFFirstVideoFrameKey = @"LFFirstVideoFrameKey";

/**
 播放器通知码
 
 - LFPlayerUnknownCode: 未知类型
 - LFPlayerPlayCompletedCode: 播放完成
 - LFPlayerPlayStartLoadingCode: 开始加载
 - LFPlayerPlayEndLoadingCode: 停止加载
 - LFPlayerPlayErrorCode: 播放错误
 - LFPlayerSeekStartCode: 快进开始
 - LFPlayerSeekStopCode: 快进结束
 - LFPlayerStartPlayCode: 开始播放
 - LFPlayerPausePlayCode: 暂停播放
 - LFPlayerStopPlayCode: 停止播放
 - LFPlayerFirstVideoFrameCode: 第一个视频帧渲染成功
 */
typedef NS_ENUM(NSUInteger, LFPlayerCode)
{
    LFPlayerUnknownCode = 0,
    LFPlayerPlayCompletedCode = 1,
    LFPlayerPlayStartLoadingCode = 2,
    LFPlayerPlayEndLoadingCode = 3,
    LFPlayerPlayErrorCode = 4,
    LFPlayerSeekStartCode = 5,
    LFPlayerSeekStopCode = 6,
    LFPlayerStartPlayCode = 7,
    LFPlayerPausePlayCode = 8,
    LFPlayerStopPlayCode = 9,
    LFPlayerFirstVideoFrameCode = 10,
};

/**
 播放器底层回调接口
 */
@protocol LFPlayerDelegate <NSObject>

@required
/**
 底层播放器向上层通知
 
 @param code 回调code类型
 */
- (void)playerNotify:(LFPlayerCode)code;

@optional
/**
 当前播放视频的size
 
 @param size 宽高
 */
- (void)playerNaturalSize:(CGSize)size;

/**
 回调当前播放进度
 
 @param playBackTime 播放的进度  单位:s
 */
- (void)playBackTime:(NSTimeInterval)playBackTime;

/**
 缓存进度

 @param prepareToPlay 缓存的进度  单位:s
 */
- (void)playableDuration:(NSTimeInterval)playableDuration;

@end

/**
 播放器抽象接口
 */
@protocol LFPlayerCore <NSObject>

@required
/**
 播放
 */
- (void)prepareToPlay;

/**
 暂停
 */
- (void)pause;

/**
 恢复播放
 */
- (void)resume;

/**
 停止(析构)
 */
- (void)stop;

/**
 是否正在播放
 
 @return BOOL
 */
- (BOOL)isPlaying;

/**
 是否正在快进
 
 @return BOOL
 */
- (BOOL)isSeeking;

/**
 播放器渲染视图
 
 @return UIView 可能为nil
 */
- (nullable UIView*)renderView;

/**
 当前这一帧
 
 @return UIImage 可能为nil
 */
- (nullable UIImage*)shotImage;

/**
 是否静音
 
 @param mute YES 静音 默认NO
 */
- (void)setMuted:(BOOL)mute;

/**
 回调delegate
 
 @param delegate 可以为nil
 */
- (void)setDelegate:(id<LFPlayerDelegate> _Nullable)delegate;

/**
 contentMode是否自适应为最大
 
 @param isFill defalut: NO
 */
- (void)setScalingAspectFill:(BOOL)isFill;

/**
 退到后台自动暂停前台自动播放
 
 @param pause default: YES
 */
- (void)setAutoPauseInBackground:(BOOL)pause;

/**
 设置自动循环播放

 @param autoRunloop autoRunloop default: NO
 */
- (void)setAutoRunloop:(BOOL)autoRunloop;

@optional
/**
 类方法创建播放器
 
 @param url 输入url 不可以为null
 @return LFPlayerCore 可能为nil
 */
+ (nullable instancetype)playerWithPlayerUrl:(NSURL *)url;

/**
 类方法创建播放器
 
 @param url 输入url 不可以为null
 @param isLive 是否为直播
 @return LFPlayerCore 可能为nil
 */
+ (nullable instancetype)playerWithPlayerUrl:(NSURL *)url isLive:(BOOL)isLive;

/**
 对象创建播放器
 
 @param url 输入url 不可以为null
 @return LFPlayerCore 可能为nil
 */
- (nullable instancetype)initWithPlayerUrl:(NSURL *)url;

/**
 对象创建播放器
 
 @param url 输入url 不可以为null
 @param isLive 是否为直播
 @return LFPlayerCore 可能为nil
 */
- (nullable instancetype)initWithPlayerUrl:(NSURL *)url isLive:(BOOL)isLive;

/**
 类方法创建播放器

 @param videoItem 对象输入(适用于rtp)
 @return LFPlayerCore
 */
+ (nullable instancetype)playerWithVideoItem:(LFVideoItem *)videoItem;

/**
 对象创建播放器

 @param videoItem 对象输入(适用于rtp)
 @return LFPlayerCore
 */
- (nullable instancetype)initWithVideoItem:(LFVideoItem *)videoItem;

/**
 快进
 
 @param currentPlaybackTime 快进的位置 单位:ms
 */
- (void)seek:(NSTimeInterval)currentPlaybackTime;

/**
 回调进度间隔
 
 @param callBackTimerInterval 回调当前时间的间隔 default: 1s
 */
- (void)setCallBackPlayTimeInterval:(NSTimeInterval)callBackTimerInterval;


/**
 当前播放url
 
 @return NSURL 可以为nil
 */
- (NSURL*)playerUrl;

/**
 播放总时长
 
 @return NSTimeInterval
 */
- (NSTimeInterval)duration;

/**
 加载的进度
 
 @return NSTimeInterval
 */
- (NSTimeInterval)playableDuration;

/**
 播放的进度
 
 @return NSTimeInterval
 */
- (NSTimeInterval)playBackTime;

/**
 帧率

 @return CGFloat
 */
- (CGFloat)videoFrameRate;

/**
 平均码率
 
 @return int64_t
 */
- (int64_t)avgvideoBitRate;

/**
 I帧平均大小
 
 @return int64_t
 */
- (int64_t)avgKeyFrameSize;

@end

NS_ASSUME_NONNULL_END

#endif /* LFPlayerProtocol_h */


