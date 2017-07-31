//
//  LFLogTestView.m
//  LFMixStreamDemo
//
//  Created by admin on 16/9/13.
//  Copyright © 2016年 admin. All rights reserved.
//

#import "LFLogTestView.h"
#import <YYKit/YYKit.h>

@interface LFLogTestView ()<UITableViewDataSource,UITableViewDelegate>

@property (nonatomic, strong) UITableView *tableView;

@end

@implementation LFLogTestView

- (instancetype)initWithFrame:(CGRect)frame{
    if(self = [super initWithFrame:frame]){
        self.backgroundColor = [UIColor clearColor];
        [self addSubview:self.tableView];
        _list = [NSMutableArray new];
    }
    return self;
}

- (void)reloadData{
    [self.tableView reloadData];
    if(self.list.count > kScreenHeight/20){
        [self.tableView scrollToBottom];
    }
}

- (UITableView*)tableView{
    if(!_tableView){
        _tableView = [UITableView new];
        _tableView.frame = self.bounds;
        _tableView.dataSource = self;
        _tableView.delegate = self;
        _tableView.backgroundColor = [UIColor clearColor];
        [_tableView registerClass:[UITableViewCell class] forCellReuseIdentifier:[UITableViewCell className]];
        [_tableView setContentInset:UIEdgeInsetsMake(100, 0, 0, 0)];
        _tableView.tableFooterView = [UIView new];
    }
    return _tableView;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section{
    return self.list.count;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath{
    return 20;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath{
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:[UITableViewCell className]];
    NSString *showMsg = [self.list objectAtIndex:indexPath.row];
    cell.backgroundColor = [UIColor colorWithWhite:0.8 alpha:0.6];
    cell.textLabel.font = [UIFont systemFontOfSize:12];
    cell.textLabel.text = showMsg;
    return cell;
}


@end
