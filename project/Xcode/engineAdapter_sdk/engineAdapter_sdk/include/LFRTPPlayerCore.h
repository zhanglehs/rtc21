//
//  LFRTPPlayerCore.h
//  LFRTPPlayerCore
//
//  Created by admin on 2016/12/29.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "LFPlayerCore.h"

/**
 播放器内核
 */
@interface LFRTPPlayerCore : NSObject<LFPlayerCore>

#pragma mark - Initializer
///=============================================================================
/// @name Initializer
///=============================================================================
- (nullable instancetype)init UNAVAILABLE_ATTRIBUTE;
+ (nullable instancetype)new UNAVAILABLE_ATTRIBUTE;

- (void)setNetworkChanged;

-(int)snapShot:(NSString *_Nullable)var1;

@end

