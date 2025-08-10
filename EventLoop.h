#pragma once
#include "noncopyable.h"
#include <functional>
#include<vector>
#include <atomic>
#include <memory>
#include <mutex>
#include "CurrentThread.h"
#include "Timestamp.h"

class Channel;
class Poller;
class TimerQueue;

//事件循环类 主要包含两个大模块， Poller和Channel
class EventLoop:Noncopyable{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();
    //开启事件循环
    void loop();
    //退出事件循环
    void quit();

    Timestamp pollReturnTime() const {return pollReturnTime_;}
    //在当前loop执行zb
    void runInLoop(Functor cb);
    //把cb放入队列中，唤醒loop所在的线程，执行cb
    void queueInLoop(Functor cb);
    //唤醒loop所在的线程
    void wakeup();
    //EventLoop的方法-->调用了Poller的方法
    void updateChannel(Channel * channel);
    void removeChannel(Channel * channel);
    bool hasChannel(Channel *channel);

    //疑问：loop不就是跑在自己的线程中的吗？     EventLoop对象是否在自己的线程中
    bool isInLoopThread() const {return threadId_ == CurrentThread::tid();}

private:
    //处理wake uo
    void handleRead();
    //执行回调
    void doPendingFunctors();

    
    using ChannelList = std::vector<Channel *>;

    std::atomic_bool looping_;//原子操作，底层通过CAS实现
    std::atomic_bool quit_;//原子操作，底层通过CAS实现
    
    //记录当前loop线程的i  d
    const pid_t  threadId_;
    //poller返回发生事件的channels的时间点
    Timestamp pollReturnTime_;
    
    std::unique_ptr<Poller> poller_;
    //当mainLoop获取一个新用户的channel，通过轮询算法选择一个subLoop，通过该成员唤醒subloop来处理channel
    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;

    std::atomic_bool callingPendingFunctors_;//标识当前loop是否有需要执行的回调操作

    //用来保证pendingFunctors_线程安全的互斥锁
    std::mutex mutex_;
    //存放loop需要执行的所有的回调操作
    std::vector<Functor> pendingFunctors_;
};