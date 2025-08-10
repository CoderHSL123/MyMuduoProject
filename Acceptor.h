#pragma once
#include <functional>

#include "noncopyable.h"
#include "Channel.h"
#include "Socket.h"


class EventLoop;
class InetAddress;





class Acceptor:Noncopyable{
    using NewConnectionCallback = std::function<void(int sockfd,const InetAddress&)>;
public:
    Acceptor(EventLoop* loop,const InetAddress& listenAddr,bool reuseport);

    ~Acceptor();

    void setNewConnectionCallBack(const NewConnectionCallback & cb){
        newConnectionCallback_ = std::move(cb);
    }

    bool listenning() const{return listenning_;}

    void listen();



private:

    void handleRead();

    //Acceptor用的就是用户定义的baseloop，也称为mainloop
    EventLoop * loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback newConnectionCallback_;
    bool listenning_;

};