#include "TcpServer.h"
#include "Logger.h"
#include <strings.h>
#include "TcpConnection.h"

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
    for(auto & item:connections_){
        TcpConnectionPtr conn(item.second);
        item.second.reset();

        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed,conn)
        );
    }
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
    // 轮询算法，选择一个subloop，来管理channel
    EventLoop* ioLoop = threadPool_->getNextLoop();

    char buf[64]={0};
    snprintf(buf,sizeof(buf),"-%s#%d",ipPort_.c_str(),nextConnId_);
    ++nextConnId_;
    std::string connName = name_+buf;

    LOG_INFO("TcpServer::newConnection [%s] - new connection [%s] from %s \n",
        name_.c_str(),connName.c_str(),peerAddr.toIpPort().c_str());
    
    //通过sockfd获取其绑定过的本机的ip地址和端口信息
    sockaddr_in local;
    ::bzero(&local,sizeof(local));

    socklen_t addrlen = sizeof(local);
    if (::getsockname(sockfd,(sockaddr*)&local,&addrlen)<0)
    {
        LOG_ERROR("sockets::getLocalAddr");
    }
    

    InetAddress localAddr(local);

    // 根据连接成功的sockfd，创建TcpConnection连接对象
    TcpConnectionPtr conn(new TcpConnection(
        ioLoop,connName,sockfd,
        localAddr,peerAddr));
    connections_[connName] = conn;
    //用户设置给TcpServer=>TcpConnection=>Channel=>Poller=>notify channel调用回调
    conn->setConnectionCallback(connectionCallback_);
    
    conn->setMessageCallback(messageCallback_);

    conn->setWriteCompleteCallback(writeCompleteCallback_);

    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection,this,std::placeholders::_1)
    );

    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished,conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn){
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));

}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn){
    
    LOG_INFO("TcpServer::removeConnectionInLoop [%s] - connection %s \n",
        name_.c_str(),conn->name().c_str());
    
    connections_.erase(conn->name());
    
    EventLoop * ioLoop = conn->getLoop();

    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed,conn)
    );

}