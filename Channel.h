#pragma once
#include "noncopyable.h"
#include <functional>
#include <memory>


/*
    封装了监听的fd fd所感兴趣的事件 如EPOLLIN、EPOLLOUT等
    同时还绑定了poller返回的具体事件
*/

class EventLoop;
class Timestamp;
class Channel:Noncopyable{
public:
    using EventCallBack = std::function<void()>;
    using ReadEventCallBack = std::function<void(Timestamp)>;

    Channel(EventLoop* loop,int fd);
    ~Channel();
    //fd得到poller通知以后，处理事件
    void handleEvent(Timestamp receiveTime);

    //设置回调函数
    void setReadCallBack(ReadEventCallBack cb){ readCallBack_ = std::move(cb);}
    void setWriteCallBack(EventCallBack cb){ writeCallBack_ = std::move(cb);}
    void setCloseCallBack(EventCallBack cb ){closeCallBack_ = std::move(cb);}
    void setErrorCallBack(EventCallBack cb ){errorCallBack_ = std::move(cb);}
    //放置当channel被手动remove掉，channel还在执行回调操作
    void tie(const std::shared_ptr<void>&);

    int fd() const{ return fd_;}
    int events() const{return events_;}
    void set_revents(int revt){revents_ = revt;}



    //设置fd响应的事件状态
    void enableReading(){events_|= kReadEvent;update();}//update的作用调用epoll_ctl把感兴趣的事件加到红黑树上

    void disableReading(){events_&=~kReadEvent;update();}

    void enableWriting(){events_|=kWriteEvent;update();}

    void disableWriting(){events_&=~kWriteEvent;update();}

    void disableAll(){events_=kNoneEvent;update();}

        //返回fd当前的事件状态
    bool isNoneEvent() const{return events_==kNoneEvent;}
    bool isWriting() const{return events_&kWriteEvent;}
    bool isReading() const{return events_&kReadEvent;}

    //用于判断该channel是否已经加入到poller中
    int index() const{return index_;}
    void set_index(int idx){index_ = idx;}

    //one loop per thread
    EventLoop* ownerLoop() const {return loop_;}
    void remove();

private:
    void update();
    //受保护的处理事件
    void handleEventWithGuard(Timestamp receiveTime);

//类内声明变量，类外初始化变量
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;
    EventLoop* loop_;//事件循环
    const int fd_;// poller监听的对象
    int events_;//fd所感兴趣的事件
    int revents_;//poller返回的发生的事件
    int index_;
    //弱智能指针，起观察作用
    std::weak_ptr<void> tie_;
    //用于判断tie_是否被绑定智能指针
    bool tied_;

    //channel通道中可以获知fd最终发生的具体的事件的revents，所以他负责具体时间的回调操作
    ReadEventCallBack readCallBack_;
    EventCallBack writeCallBack_;
    EventCallBack closeCallBack_;
    EventCallBack errorCallBack_;
};