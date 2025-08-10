#include "Acceptor.h"
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>


#include "Logger.h"
#include "InetAddress.h"

static int createNonblocking(){
    int sockfd = ::socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);
    if (sockfd)
    {
        LOG_FATAL("%s : %s : %d listen socket create err:%d \n",__FILE__,__FUNCTION__,__LINE__,errno);
    }
    

    return sockfd;
}

Acceptor::Acceptor(EventLoop* loop,const InetAddress& listenAddr,bool reuseport)
:loop_(loop),acceptSocket_(createNonblocking()),
acceptChannel_(loop_,acceptSocket_.fd()),listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);
    acceptSocket_.bindAddress(listenAddr);
    //TcpServer::start() Acceptor::listen 有新用户的连接，要执行一个回调(connfd -> channel->subloop)
    acceptChannel_.setReadCallBack(
        std::bind(&Acceptor::handleRead,this));
    
}

Acceptor::~Acceptor(){
    //清除该channel的所有感兴趣的事件
    acceptChannel_.disableAll();
    //将自己从poller中删除
    acceptChannel_.remove();
}


void Acceptor::listen(){
    listenning_=true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();

}
//listenfd有新用户连接事件发生 就会在该函数执行回调函数
void Acceptor::handleRead(){
    InetAddress peerAddr;
    int connfd = acceptSocket_.accept(&peerAddr);
    if (connfd>=0)
    {

        if (newConnectionCallback_)
        {
            //该回调函数作用 使用轮询方式将新建立的连接添加到subreactor中
            newConnectionCallback_(connfd,peerAddr);
        }else{
            ::close(connfd);
        }
        
    }else{
        LOG_ERROR("%s : %s : %d  accept err:%d \n",__FILE__,__FUNCTION__,__LINE__,errno);
        //EMFILE表示可用文件描述符的数量到达上限
        if (errno==EMFILE)
        {
            LOG_ERROR("%s : %s : %d  sockfd reached limit! \n",__FILE__,__FUNCTION__,__LINE__);
        }
    }
    

}