#include "Thread.h"
#include "CurrentThread.h"


//类内定义类外初始化
std::atomic_int32_t Thread::numCreated_ = {0};

Thread::Thread(ThreadFunc func,const std::string& name)
:started_(false),joined_(false),
tid_(0),func_(std::move(func)),
name_(name)
{
    setDefaultName();
}
Thread::~Thread(){
    if (started_&&!joined_)
    {
        thread_->detach();
    }
    
}

void Thread::start(){
    started_ =true;



    //开启线程
    thread_ = std::make_shared<std::thread>([&](){
        //获取线程的tid值
        tid_ = CurrentThread::tid();
        {
            std::unique_lock<std::mutex> lock(tidMutex_);
            tidCond_.notify_one();
        }
        func_();//线程专门执行该线程函数
    });

    //这里必须等待上面新创建的线程给tid_赋值
    std::unique_lock<std::mutex> lock(tidMutex_);
    tidCond_.wait(lock);
}

void Thread::join(){
    thread_->join();
    joined_=true;
}
//生成默认名字 Thread+numCreated_
void Thread::setDefaultName(){
    int num = ++numCreated_;
    if (name_.empty())
    {
        char buf[32]={0};
        snprintf(buf,sizeof buf,"Thread%d",num);
        name_ = buf;
    }
    
}