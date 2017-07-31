//
//  ViewController.m
//  LFMixStreamDemo
//
//  Created by admin on 16/9/12.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "PlayerDelegate.h"

@implementation PlayerDelegate{
    int playerid_;
    id<PlayerCallbackDelegate> delegate_;
}

-(void) setDelegate:(id<PlayerCallbackDelegate>)delegate
{
    delegate_ = delegate;
}

-(void) setPlayerId:(int)playerId
{
    playerid_ = playerId;
}

- (void)playerNotify:(LFPlayerCode)code
{
    if(delegate_ != nil)
        [delegate_ playerNotify:code playerId:playerid_];
}

/**
 当前播放视频的size
 
 @param size 宽高
 */
- (void)playerNaturalSize:(CGSize)size
{
    NSLog(@"playerid_:%d,size:[%f,%f]\n",playerid_,size.width,size.height);
    CGSize changeSize = CGSizeMake(size.width * 2/5, size.height *2/5);
    if(delegate_ != nil)
       [delegate_ playerNaturalSize:changeSize playerId:playerid_];
}

/**
 回调当前播放进度
 
 @param playBackTime 播放的进度  单位:s
 */
- (void)playBackTime:(NSTimeInterval)playBackTime
{
    NSLog(@"playerid_:%d,size:%f\n",playerid_,playBackTime);
}

/**
 缓存进度
 
 @param prepareToPlay 缓存的进度  单位:s
 */
- (void)playableDuration:(NSTimeInterval)playableDuration
{
    NSLog(@"playerid_:%d,size:%f\n",playerid_,playableDuration);
}

@end
