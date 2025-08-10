#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
            const std::string& name)
            :loop_(nullptr),
            exiting_(false),
            thread_(std::bind(&EventLoopThread::threadFunc,this),name),
             mutex_(),
             cond_(),
             callback_(cb)
            {

            }
EventLoopThread::~EventLoopThread(){
        exiting_=true;
        if(loop_!=nullptr)
        {
                loop_->quit();
                thread_.join();
        }
}
EventLoop * EventLoopThread::startLoop(){
        //启动底层新线程
        thread_.start();
        //主线程等待线程给loop_赋值
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock,[this](){
                return (loop_!=nullptr);
        });
        
        //执行到此处表明loop_不为空
        return loop_;
}

//底层线程执行的方法
void EventLoopThread::threadFunc(){
        EventLoop loop;//创建 一个独立的EventLoop，和上面的线程一一对应，one loop per thread

        if(callback_){
                callback_(&loop);
        }
        
        {
                std::lock_guard<std::mutex> lk(mutex_);
                loop_ = &loop;
                cond_.notify_one();
        }
        //poller开始进行wait等待事件触发
        loop.loop();
        std::lock_guard<std::mutex> lock(mutex_);
        loop_=nullptr;
}