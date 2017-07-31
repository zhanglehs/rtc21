
//  Created by admin on 16/9/12.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "MainViewController.h"
#import <YYKit/YYKit.h>
#import <YYKit/YYReachability.h>

#import "InitViewController.h"
#import "ViewController.h"

@interface MainViewController ()

@property (nonatomic,strong) InitViewController *laifengInitVC;
@property (nonatomic,strong) ViewController *laifengRtpVC;
@property (nonatomic, strong) UIViewController *currentVC;
@property (nonatomic, strong) UIView *contentView;
@end

@implementation MainViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    [self setupSubControllers];
}

- (void)viewDidAppear:(BOOL)animated{
    [super viewDidAppear:animated];
    [UIApplication sharedApplication].idleTimerDisabled = YES;
}

- (void)viewDidDisappear:(BOOL)animated{
    [super viewDidDisappear:animated];
    [UIApplication sharedApplication].idleTimerDisabled = NO;
}

-(void)setupSubControllers{
    _laifengRtpVC = [[ViewController alloc] init];
    [self addChildViewController:_laifengRtpVC];
    [self.view addSubview:_laifengRtpVC.view];
    
    _laifengInitVC = [[InitViewController alloc] init];
    [self addChildViewController:_laifengInitVC];
    [_laifengInitVC setParentViewController:self];
    
    [self fitFrameForChildViewController:_laifengInitVC];
    [self.view addSubview:_laifengInitVC.view];
    self.currentVC = _laifengInitVC;
}


#pragma mark -- Setter Getter
-(UIView *)contentView{
    if(!_contentView){
        _contentView = [UIButton new];
        CGSize size = [[UIScreen mainScreen] bounds].size;
        CGPoint point = [[UIScreen mainScreen] bounds].origin;
        _contentView.size = size;
        _contentView.top = point.y;
        _contentView.left = point.x;
        [_contentView setBackgroundColor:[UIColor blueColor]];
    }
    return _contentView;
}

- (void)transitionFromOldViewController:(UIViewController *)oldViewController toNewViewController:(UIViewController *)newViewController{
    
    [self transitionFromViewController:oldViewController toViewController:newViewController duration:0.3 options:UIViewAnimationOptionTransitionCrossDissolve animations:nil completion:^(BOOL finished) {
        if (finished) {
            [newViewController didMoveToParentViewController:self];
            _currentVC = newViewController;
        }else{
            _currentVC = oldViewController;
        }
    }];
}

- (void)fitFrameForChildViewController:(UIViewController *)chileViewController{
    CGRect frame = self.view.frame;
    frame.origin.y = 0;
    chileViewController.view.frame = frame;
}

- (void)removeAllChildViewControllers{
    for (UIViewController *vc in self.childViewControllers) {
        [vc willMoveToParentViewController:nil];
        [vc removeFromParentViewController];
    }
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)transController:(int)nameId
{
    if(nameId == 0){
        [self fitFrameForChildViewController:_laifengInitVC];
        [UIView animateWithDuration:0.3 animations:^{
            self.laifengInitVC.view.left = 0;
            self.laifengRtpVC.view.left = self.laifengRtpVC.view.width * -1;
            [self.view bringSubviewToFront:self.laifengInitVC.view];
        }];
    }
    else if(nameId == 5){
        [self fitFrameForChildViewController:_laifengRtpVC];
        [UIView animateWithDuration:0.3 animations:^{
            self.laifengInitVC.view.left = self.laifengInitVC.view.width * -1;
            self.laifengRtpVC.view.left = 0;
            [self.view bringSubviewToFront:self.laifengRtpVC.view];
        }];
    }
}

@end
