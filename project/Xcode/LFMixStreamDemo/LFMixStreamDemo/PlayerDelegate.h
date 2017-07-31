//
//  ViewController.h
//  LFMixStreamDemo
//
//  Created by admin on 16/9/12.
//  Copyright © 2016年 admin. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <YYKit/YYKit.h>
#import "LFPlayerTestView.h"
#import "../../engineAdapter_sdk/engineAdapter_sdk/include/LFRTPPlayerCore.h"
#import "LFLogTestView.h"
#import <YYKit/YYReachability.h>


@protocol PlayerCallbackDelegate <NSObject>

- (void)playerNotify:(LFPlayerCode)code
            playerId:(int)playerId;

- (void)playerNaturalSize:(CGSize)size
            playerId:(int)playerId;
@end

@interface PlayerDelegate : NSObject<LFPlayerDelegate>

-(void) setDelegate:(id<PlayerCallbackDelegate>)delegate;
-(void) setPlayerId:(int)playerId;

@end

