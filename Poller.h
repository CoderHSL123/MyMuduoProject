#pragma once
#include "noncopyable.h"
#include <vector>
#include <unordered_map>

class Channel;
class EventLoop;

class Timestamp;
//muduo苦衷多路事件分发器的核心IO复用模块
class Poller:Noncopyable{
public:
    using ChannelList = std::vector<Channel*>;

    //构造函数
    Poller(EventLoop* loop);
    //虚析构函数
    virtual ~Poller();

    //给所有IO复用保留统一的接口
    virtual Timestamp poll(int timeoutMs,ChannelList* activeChannels) = 0;

    virtual void updateChannel(Channel*channel)=0;

    virtual void removeChannel(Channel*channel)=0;
    //判断channel是否在当前poller中
    bool hasChannel(Channel* channel) const;
    //EventLoop可以通过该接口获取默认的IO复用的具体事件
    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    // map的key：sockfd value:fd所属的channel
    using ChnnelMap = std::unordered_map<int,Channel*>;
    ChnnelMap channels_;
private:
    EventLoop* ownerLoop_;

};