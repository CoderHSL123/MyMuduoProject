#include "EPollPoller.h"
#include "Logger.h"
#include <errno.h>
#include <unistd.h>
#include "Timestamp.h"
#include "Channel.h"
#include <strings.h>

//channel未添加到poller中
const int kNew = -1;//channel的成员index_ = -1
//channel已添加到poller中
const int kAdded = 1;
//channel从poller中删除
const int kDeleted = 2;

EPollPoller::EPollPoller(EventLoop* loop)
:Poller(loop),
epollfd_(epoll_create1(EPOLL_CLOEXEC)),
events_(kInitEventListSize_)
{
    if(epollfd_<0){
        LOG_FATAL("epoll_create error:%d \n",errno);
    }
}

EPollPoller::~EPollPoller(){
    ::close(epollfd_);
}


//给所有IO复用保留统一的接口
Timestamp EPollPoller::poll(int timeoutMs,ChannelList* activeChannels){
    LOG_INFO("func =%s fd total count =%lu \n",__FUNCTION__,activeChannels->size());
    //&(*events_.begin())  相当于拿到了vector底层数组的起始地址
    int numEvents = ::epoll_wait(epollfd_,&(*events_.begin()),events_.size(),timeoutMs);
    int saveErrno = errno;
    Timestamp now = Timestamp::now();
    //numEvents>0 说明有事件触发
    if(numEvents>0){
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannels(numEvents,activeChannels);
        //返回事件的个数与events_监听的数量相同 则需要扩大events_的容量 ？？？ events_不应该是往里面加需要监听的事件才扩容吗
        if (numEvents==events_.size())
        {
            events_.resize(events_.size()*2);
        }
        
    }else if(numEvents==0){
        LOG_DEBUG("%s timeout \n",__FUNCTION__);

    }else{
        //如果saveErrno不等于外部中断
        if (saveErrno!=EINTR)
        {
            LOG_ERROR("EPollPoller::poll() error:%d \n",saveErrno);
        }
        
    }
    return now;
}

//channel update/remove --> EventLoop updateChannel/removeChannel => Poller updateChannel
void EPollPoller::updateChannel(Channel*channel){
    //
    const int index = channel->index();
    LOG_INFO("func =%s fd=%d, events=%d index=%d \n",__FUNCTION__,channel->fd(),channel->events(),index);
    if(index==kNew||index==kDeleted){
        if(index==kNew){
            channels_[channel->fd()] = channel;
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }else{
        //此channel处于监听列表中
        int fd = channel->fd();
        //判断channel是否设置了事件
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);
        }else{
            update(EPOLL_CTL_MOD,channel);
        }
        

    }
}
//从poller中删除channel
void EPollPoller::removeChannel(Channel*channel){
    LOG_INFO("func =%s fd=%d\n",__FUNCTION__,channel->fd());
    const int fd = channel->fd();
    //将fd从channels_中移除
    channels_.erase(fd);
    
    int index = channel->index();
    //如果现在该fd还在epoll的红黑树上 即kAdded状态 则从epoll上删除
    if (index==kAdded)
    {
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_index(kNew);

}


void EPollPoller::fillActiveChannels(int numEvents,ChannelList* activeChannels) const{
    for(int i=0;i<numEvents;++i){
        //events_就是所有触发的事件的列表
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        //EventLoop拿到了他的poller给他返回的所有发生时间的channel列表
        activeChannels->push_back(channel);
    }
}
    //更新channel通道
void EPollPoller::update(int operation, Channel* channel){
    epoll_event event;
    bzero(&event,sizeof(event));
    event.events = channel->events();
    event.data.fd = channel->fd();
    event.data.ptr = channel;
    int fd = channel->fd();
    if(::epoll_ctl(epollfd_,operation,fd,&event)<0){
        if (operation==EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d \n",errno);
        }else{
            LOG_FATAL("epoll_ctl add/mod error:%d \n",errno);
        }
        
    }
}