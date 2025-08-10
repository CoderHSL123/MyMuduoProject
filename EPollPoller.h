#pragma once
#include <sys/epoll.h>
#include "Poller.h"
#include <vector>

/*
    epoll的使用
    epoll_create
    epoll_ctl add/mod/del
    epoll_wait

*/

class Channel;

class EPollPoller : public Poller{
public:
    EPollPoller(EventLoop* loop);
    ~EPollPoller();
    
    //给所有IO复用保留统一的接口
    Timestamp poll(int timeoutMs,ChannelList* activeChannels) override;

    void updateChannel(Channel*channel) override;

    void removeChannel(Channel*channel) override;

private:
    //EventList初始大小
    static const int kInitEventListSize_ = 16;
    //返回触发的事件的channel到activeChannels
    void fillActiveChannels(int numEvents,ChannelList* activeChannels) const;
    //更新channel通道
    void update(int operation, Channel* channel);



    using EventList = std::vector<epoll_event>;
    //epoll的红黑树根节点的文件描述符
    int epollfd_;
    //触发的事件列表
    EventList events_;
};