#pragma once
#include <memory>
#include <string>
#include <atomic>


#include "noncopyable.h"
#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
#include "Timestamp.h"

class Channel;
class EventLoop;
class Socket;

/*
    TcpServer->TcpConnection->Channel->Poller->Channel的回调

*/


class TcpConnection:Noncopyable,public std::enable_shared_from_this<TcpConnection>{
public:
    TcpConnection(EventLoop* loop,
                    const std::string &name,
                    int sockfd,
                    const InetAddress& localAddr,
                    const InetAddress& peerAddr);
    ~TcpConnection();

    EventLoop* getLoop() const{return loop_;}

    const std::string& name() const {return name_;}

    const InetAddress& localAddress() const{ return localAddr_;}
    const InetAddress& peerAddress() const { return peerAddr_;}

    bool connected() const {return state_==StateE::kConnected;}
    bool disConnected() const {return state_==StateE::kDisconnected;}
    //发送数据
    void send(const void* message,int len);
    //关闭连接
    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb){ connectionCallback_ = std::move(cb);}
    void setMessageCallback(const MessageCallback& cb){ messageCallback_ = std::move(cb);}
    void setCloseCallback(const CloseCallback& cb){ closeCallback_ = std::move(cb);}
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,size_t highWaterMark)
    { highWaterMarkCallback_ = std::move(cb); highWaterMark_=highWaterMark_;}
    //连接建立
    void connectEstablished();
    //连接销毁
    void connectDestroyed();

    
private:
    enum StateE{kDisconnected,kConnecting,kConnected,kDisconnecting};

    void setState(StateE state) {state_ = state;}

    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    void sendInLoop();

    void shutdownInLoop();


    EventLoop * loop_;//TcpConnection的loop是运行在mainreactor(单reactor)或者subreactor(多reactor)中
    const std::string name_;

    std::atomic_int state_;
    bool reading_;
    
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    const InetAddress localAddr_;//记录本机地址和端口
    const InetAddress peerAddr_;//客户端地址和端口


    //执行连接关闭的回调
    ConnectionCallback connectionCallback_;
    //有读写消息时的回调
    MessageCallback messageCallback_;
    //消息写完成后的回调
    WriteCompleteCallback writeCompleteCallback_;

    HighWaterMarkCallback highWaterMarkCallback_;
    //关闭连接执行的回调
    CloseCallback closeCallback_;

    size_t highWaterMark_;

    Buffer inputBuffer_;

    Buffer outputBuffer_;
};