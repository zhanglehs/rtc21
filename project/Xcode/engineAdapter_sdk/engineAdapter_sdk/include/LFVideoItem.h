//
//  LFVideoItem.h
//  LFRTPPlayerCore
//
//  Created by admin on 2017/2/17.
//  Copyright © 2017年 admin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface LFVideoItem : NSObject

@property (nonatomic, copy, nullable) NSString *alias;      ///< 别名
@property (nonatomic, copy, nullable) NSString *appId;      ///< 101
@property (nonatomic, copy, nullable) NSString *token;      ///< 98765
@property (nonatomic, assign) CGSize videoSize;
@property (nonatomic, copy) NSString * _Nullable hosturl;

@end
