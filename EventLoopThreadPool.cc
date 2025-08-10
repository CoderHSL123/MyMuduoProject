#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "EventLoop.h"


EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, const std::string &nameArg)
:baseLoop_(baseLoop),name_(nameArg),
started_(false),numThreads_(0),
next_(0)
{
    
}

EventLoopThreadPool::~EventLoopThreadPool(){

}


void EventLoopThreadPool::start(const ThreadInitCallback &cb){
    started_=true;

/*

 EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                            const std::string& name = std::string());
*/

    for(int i= 0;i<numThreads_;i++){
        char buf[name_.size()+32]={0};
        snprintf(buf,sizeof(buf),"%s%d",name_.c_str(),i);
        std::unique_ptr<EventLoopThread> eventLoopThreadPtr = std::make_unique<EventLoopThread>(cb,std::string(buf));
        threads_.push_back(std::move(eventLoopThreadPtr));
        //底层创建线程，绑定一个新的EventLoop，并返回该eventloop的地址
        loops_.push_back(eventLoopThreadPtr->startLoop());
    }
    //整个服务端只有一个线程，运行着baseloop
    if(numThreads_==0&&cb){
        cb(baseLoop_);
    }
}

//如果是工作在多线程中，baseLoop_默认以轮询的方式分配channel给subloop
EventLoop * EventLoopThreadPool::getNextLoop(){
    //subreactor为空
    if (loops_.empty())
    {
        return baseLoop_;
    }
    //通过轮询的方式从子线程中取loop
    EventLoop* loop  = loops_[next_++];
    if (next_>=loops_.size())
    {
        next_=0;
    }
    
    return loop;
    
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops(){
    //如果没有subreactor 返回mainReactor
    if(loops_.empty()){
        return std::vector<EventLoop*>(1,baseLoop_);
    }else{
        return loops_;
    }
}
