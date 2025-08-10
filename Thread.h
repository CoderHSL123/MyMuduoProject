#pragma once
#include <functional>
#include <thread>
#include <memory>
#include <unistd.h>
#include <string>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include "noncopyable.h"

//一个Thread类记录的就是一个线程的详细信息
class Thread : Noncopyable{
    using ThreadFunc = std::function<void()>;
public:
    explicit Thread(ThreadFunc ,const std::string& name= std::string());
    ~Thread();

    void start();
    void join();

    bool started() const { return started_; }
    pid_t tid()const{return tid_;}
    const std::string& name() const{return name_;}

    static int numCreated(){return numCreated_;}


private:
    void setDefaultName();

    bool started_;
    bool joined_;
    std::shared_ptr<std::thread> thread_;
    pid_t tid_;
    ThreadFunc func_;
    std::string name_;
    //产生线程的个数
    static std::atomic_int32_t numCreated_;
    //保证tid_能够正常的赋值的锁
    std::mutex tidMutex_;
    std::condition_variable tidCond_;
};