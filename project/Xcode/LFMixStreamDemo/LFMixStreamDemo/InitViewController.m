
//  Created by admin on 16/9/12.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "InitViewController.h"
#import <YYKit/YYKit.h>
#import <YYKit/YYReachability.h>

#import "ViewController.h"

@interface InitViewController ()

@property (nonatomic,strong) UIButton *LaifengTestLianTong;
@property (nonatomic,strong) UIButton *LaifengTestDownloadHuabei;
@property (nonatomic,strong) UIButton *LaifengTestDownloadHuadong;
@property (nonatomic,strong) UIButton *LaifengTestDownloadHuanan;
@property (nonatomic,strong) UIButton *LaifengTestRtp;
@property (nonatomic,strong) UIButton *LaifengTestLianMic;

@property (nonatomic,strong) MainViewController *parentVC;
@end

@implementation InitViewController

-(void) setParentViewController:(MainViewController *)pVC
{
    self.parentVC = pVC;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    [self.view addSubview:self.LaifengTestLianTong];
    [self.view addSubview:self.LaifengTestDownloadHuabei];
    [self.view addSubview:self.LaifengTestDownloadHuadong];
    [self.view addSubview:self.LaifengTestDownloadHuanan];
    [self.view addSubview:self.LaifengTestRtp];
    [self.view addSubview:self.LaifengTestLianMic];
    
    NSLog(@"init view controller load\n");
}

- (void)viewDidAppear:(BOOL)animated{
    [super viewDidAppear:animated];
    [UIApplication sharedApplication].idleTimerDisabled = YES;
}

- (void)viewDidDisappear:(BOOL)animated{
    [super viewDidDisappear:animated];
    [UIApplication sharedApplication].idleTimerDisabled = NO;
}

#pragma mark -- Setter Getter
-(UIButton *)LaifengTestLianTong{
    if(!_LaifengTestLianTong){
        _LaifengTestLianTong = [UIButton new];
        CGSize size = [[UIScreen mainScreen] bounds].size;
        CGPoint point = [[UIScreen mainScreen] bounds].origin;
        _LaifengTestLianTong.size = CGSizeMake(size.width/2, size.height/3);
        _LaifengTestLianTong.top = point.y;
        _LaifengTestLianTong.left = point.x;
        [_LaifengTestLianTong setTitle:@"连接性" forState:UIControlStateNormal];
        [_LaifengTestLianTong setBackgroundColor:[UIColor blueColor]];
    }
    return _LaifengTestLianTong;
}

-(UIButton *)LaifengTestDownloadHuabei{
    if(!_LaifengTestDownloadHuabei){
        _LaifengTestDownloadHuabei = [UIButton new];
        CGSize size = [[UIScreen mainScreen] bounds].size;
        CGPoint point = [[UIScreen mainScreen] bounds].origin;
        _LaifengTestDownloadHuabei.size = CGSizeMake(size.width/2, size.height/3);
        _LaifengTestDownloadHuabei.top = point.y;
        _LaifengTestDownloadHuabei.left = point.x + size.width/2;
        [_LaifengTestDownloadHuabei setTitle:@"华北" forState:UIControlStateNormal];
        [_LaifengTestDownloadHuabei setBackgroundColor:[UIColor greenColor]];
    }
    return _LaifengTestDownloadHuabei;
}

-(UIButton *)LaifengTestDownloadHuadong{
    if(!_LaifengTestDownloadHuadong){
        _LaifengTestDownloadHuadong = [UIButton new];
        CGSize size = [[UIScreen mainScreen] bounds].size;
        CGPoint point = [[UIScreen mainScreen] bounds].origin;
        _LaifengTestDownloadHuadong.size = CGSizeMake(size.width/2, size.height/3);
        _LaifengTestDownloadHuadong.top = point.y+size.height/3;
        _LaifengTestDownloadHuadong.left = point.x;
        [_LaifengTestDownloadHuadong setTitle:@"华东" forState:UIControlStateNormal];
        [_LaifengTestDownloadHuadong setBackgroundColor:[UIColor orangeColor]];
    }
    return _LaifengTestDownloadHuadong;
}

-(UIButton *)LaifengTestDownloadHuanan{
    if(!_LaifengTestDownloadHuanan){
        _LaifengTestDownloadHuanan = [UIButton new];
        CGSize size = [[UIScreen mainScreen] bounds].size;
        CGPoint point = [[UIScreen mainScreen] bounds].origin;
        _LaifengTestDownloadHuanan.size = CGSizeMake(size.width/2, size.height/3);
        _LaifengTestDownloadHuanan.top = point.y+size.height/3;
        _LaifengTestDownloadHuanan.left = point.x+size.width/2;
        [_LaifengTestDownloadHuanan setTitle:@"华南" forState:UIControlStateNormal];
        [_LaifengTestDownloadHuanan setBackgroundColor:[UIColor yellowColor]];
    }
    return _LaifengTestDownloadHuanan;
}

-(UIButton *)LaifengTestRtp{
    if(!_LaifengTestRtp){
        _LaifengTestRtp = [UIButton new];
        CGSize size = [[UIScreen mainScreen] bounds].size;
        CGPoint point = [[UIScreen mainScreen] bounds].origin;
        _LaifengTestRtp.size = CGSizeMake(size.width/2, size.height/3);
        _LaifengTestRtp.top = point.y+size.height/3+size.height/3;
        _LaifengTestRtp.left = point.x;
        [_LaifengTestRtp setTitle:@"Laifeng + Rtp" forState:UIControlStateNormal];
        @weakify(self)
        [_LaifengTestRtp addBlockForControlEvents:UIControlEventTouchUpInside block:^(id  _Nonnull sender) {
            @strongify(self)
            [self.parentVC transController:5];
        }];
        _LaifengTestRtp.accessibilityIdentifier = @"testRtp";
        [_LaifengTestRtp setBackgroundColor:[UIColor redColor]];
    }
    return _LaifengTestRtp;
}

-(UIButton *)LaifengTestLianMic{
    if(!_LaifengTestLianMic){
        _LaifengTestLianMic = [UIButton new];
        CGSize size = [[UIScreen mainScreen] bounds].size;
        CGPoint point = [[UIScreen mainScreen] bounds].origin;
        _LaifengTestLianMic.size = CGSizeMake(size.width/2, size.height/3);
        _LaifengTestLianMic.top = point.y+size.height/3+size.height/3;
        _LaifengTestLianMic.left = point.x+size.width/2;
        [_LaifengTestLianMic setTitle:@"连麦" forState:UIControlStateNormal];
        [_LaifengTestLianMic setBackgroundColor:[UIColor purpleColor]];
    }
    return _LaifengTestLianMic;
}

@end
