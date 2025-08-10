#include "Channel.h"
#include <sys/epoll.h>
#include "EventLoop.h"
#include "Timestamp.h"
#include "Logger.h"


const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN|EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop,int fd)
:loop_(loop),fd_(fd),events_(0),
revents_(0),index_(-1),tied_(false)
{

}

Channel::~Channel(){

}

//QUESTION channel的tie方法是什么时候调用的呢？
void Channel::tie(const std::shared_ptr<void>& obj){
    tie_= obj;
    tied_=true;
}
/*
当改变channel所表示的fd的事件后，update负责在poller里面更改fd相应的时间epoll_ctl
*/
void Channel::update(){
    //通过channel对应的EventLoop对象，调用poller的对象，注册fd的事件
    //add code...
    //TOTO...
    loop_->updateChannel(this);
}

//在channel对应的eventloop中，将channel从poller、channellist中移除
void Channel::remove(){
    //TODO add code...
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime){
    std::shared_ptr<void> guard;
    if (tied_)
    {
        //lock方法 如果tie_还存在则返回std::shared_ptr对象，否则返回nullptr
        guard = tie_.lock();
        if (guard)
        {
            handleEventWithGuard(receiveTime);
        }
        
    }else{
        handleEventWithGuard(receiveTime);
    }
    
}

//根据poller通知的channel发生的具体事件，由channel负责的事件的回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime){
    LOG_INFO("channel handleEvent revents:%d\n",revents_);

    if ((revents_&EPOLLHUP)&&(revents_&EPOLLIN))
    {
        if (closeCallBack_)
        {
            closeCallBack_();
        }
        
    }

    if (revents_& EPOLLERR )
    {
        if (errorCallBack_)
        {
            errorCallBack_();
        }
    }

    if (revents_&EPOLLIN)
    {
        if(readCallBack_){

            readCallBack_(receiveTime);
        }
    }
    if(revents_&EPOLLOUT){
        if(writeCallBack_){
            writeCallBack_();
        }
    }
    
}
