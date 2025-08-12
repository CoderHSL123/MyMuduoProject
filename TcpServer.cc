#include "TcpServer.h"
#include "Logger.h"

static EventLoop * CheckLoiopNotNull(EventLoop* loop){
    if (loop==nullptr)
    {
        LOG_FATAL("%s : %s : %d mainloop is null!\n",__FILE__,__FUNCTION__,__LINE__);
    }
    
    return loop;
}


TcpServer::TcpServer(EventLoop* loop,const InetAddress& listenAddr,
                const std::string& nameArg,Option option)
                :loop_(CheckLoiopNotNull(loop)),ipPort_(listenAddr.toIpPort()),
                name_(nameArg),
                acceptor_(new Acceptor(loop, listenAddr, option==Option::kReusePort)),
                threadPool_(new EventLoopThreadPool(loop,name_)),
                connectionCallback_(),
                messageCallback_(),
                nextConnId_(1)
{
    
    acceptor_->setNewConnectionCallBack(std::bind(&TcpServer::newConnection,this,
        std::placeholders::_1,std::placeholders::_2));
}

TcpServer::~TcpServer()
{

}


//设置底层subloop的个数
void TcpServer::setThreadNum(int numThreads){

}

//开启服务器监听
void TcpServer::start(){
    //放置一个TcpServer对象被重复启动多次
    if (started_++==0)
    {
        //启动底层线程池
        threadPool_->start(threadInitCallback_);
        //将acceptor的channel挂到mainreactor的epoll上
        loop_->runInLoop(std::bind(&Acceptor::listen,acceptor_.get()));
        //开始监听acceptor
        loop_->loop();
    }
    
}



void TcpServer::newConnection(int sockfd,const InetAddress& peerAddr){

}

void TcpServer::removeConnection(const TcpConnectionPtr& conn){

}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn){

}