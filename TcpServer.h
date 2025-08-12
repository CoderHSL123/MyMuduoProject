#pragma once
#include <functional>
#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>

#include "noncopyable.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "EventLoopThreadPool.h"
#include "Callbacks.h"

//对外服务器编程使用的类

class TcpServer:Noncopyable
{
    //using ThreadInitCallback = std::function<void(EventLoop*)>;
    using ThreadInitCallback = std::function<void(EventLoop*)>;
public:
    enum Option{
        kNoReusePort,
        kReusePort,
    };
    TcpServer(EventLoop*loop,const InetAddress& listenAddr,
                const std::string& nameArg,Option option=Option::kNoReusePort);
    ~TcpServer();
    
    void setThreadInitCallback(const ThreadInitCallback &cb){ threadInitCallback_ = std::move(cb); }

    void setConnectionCallback(const ConnectionCallback&cb){connectionCallback_ = std::move(cb); }
    
    void setMessageCallback(const MessageCallback& cb){messageCallback_ = std::move(cb);}

    void setWriteCompleteCallback(const WriteCompleteCallback& cb){ writeCompleteCallback_ = std::move(cb);}

    //设置底层subloop的个数
    void setThreadNum(int numThreads);
    
    //开启服务器监听
    void start();

private:
    void newConnection(int sockfd,const InetAddress& peerAddr);

    void removeConnection(const TcpConnectionPtr& conn);

    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    



    using ConnectionMap = std::unordered_map<std::string,TcpConnectionPtr>;

    EventLoop * loop_; //baseLoop 用户定义的loop
    const std::string ipPort_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_; //运行在mainLoop，任务就是监听新连接事件
    std::shared_ptr<EventLoopThreadPool> threadPool_; //one loop per thread

    //有新连接时的回调
    ConnectionCallback connectionCallback_;
    //有读写消息时的回调
    MessageCallback messageCallback_;
    //消息写完成后的回调
    WriteCompleteCallback writeCompleteCallback_;
    //线程初始化的回调
    ThreadInitCallback threadInitCallback_;

    std::atomic_int started_;

    int nextConnId_;
    //保存所有的连接
    ConnectionMap connections_;
};