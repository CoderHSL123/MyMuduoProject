#pragma once
#include <functional>
#include <string>
#include <vector>
#include <memory>

#include "noncopyable.h"



class EventLoop;
class EventLoopThread;


class EventLoopThreadPool:Noncopyable{
    using ThreadInitCallback = std::function<void(EventLoop*)>;
public:
    EventLoopThreadPool(EventLoop* baseLoop, const std::string &nameArg);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads){ numThreads_ = numThreads;}
   
    void start(const ThreadInitCallback &cb=ThreadInitCallback());

   //如果是工作在多线程中，baseLoop_默认以轮询的方式分配channel给subloop
   EventLoop * getNextLoop();

   std::vector<EventLoop*> getAllLoops();

   bool started() const{ return started_;}

   const std::string & name() const{ return name_;}

private:

    EventLoop * baseLoop_;//EventLoop loop
    std::string name_;
    int numThreads_;
    int next_;
    bool started_;

    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    //loops_中的EventLoop*指针是在线程启动的时候在栈上创建的，所以在析构线程池的时候不需要手动释放资源
    std::vector<EventLoop*> loops_;
};
