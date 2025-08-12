#include <functional>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>

#include "TcpConnection.h"
#include "Logger.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"


static EventLoop * CheckLoiopNotNull(EventLoop* loop){
    if (loop==nullptr)
    {
        LOG_FATAL("%s : %s : %d mainloop is null!\n",__FILE__,__FUNCTION__,__LINE__);
    }
    
    return loop;
}


TcpConnection::TcpConnection(EventLoop* loop, const std::string &name,
                int sockfd, const InetAddress& localAddr, const InetAddress& peerAddr)
                :loop_(CheckLoiopNotNull(loop)),name_(name),state_(kConnecting),
                reading_(true),socket_(new Socket(sockfd)),channel_(new Channel(loop,sockfd)),
                localAddr_(localAddr),peerAddr_(peerAddr),highWaterMark_(64*1024*1024)//64M
                {
                    //给channel设置相应的回调函数，poller给channel通知感兴趣的事件发生了，会调用相应的回调函数
                    channel_->setReadCallBack(std::bind(&TcpConnection::handleRead,this,std::placeholders::_1));
                    channel_->setWriteCallBack(std::bind(&TcpConnection::handleWrite,this));
                    channel_->setCloseCallBack(std::bind(&TcpConnection::handleClose,this));
                    channel_->setErrorCallBack(std::bind(&TcpConnection::handleError,this));
                    LOG_INFO("TcpConnection::ctor[%s] at fd = %d \n",name_.c_str(),sockfd);
                    //启动Tcp保活机制
                    socket_->setKeepAlive(true);

                }

TcpConnection::~TcpConnection(){
    LOG_INFO("TcpConnection::dtor[%s] at fd = %d state = %d \n",name_.c_str(),channel_->fd(),state_.load());
}


void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(),&savedErrno);
    if (n>0)
    {
        //已建立连接的用户，有可读事件发生了，调用用户传入的回调操作onMessage
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }else if (n==0)
    {
        //==0表示关闭连接
        handleClose();
    }else{
        //出错
        errno=savedErrno;
        LOG_ERROR("TcpConnection::handleRead");
        handleError();
    }
    
    
}

void TcpConnection::handleWrite()
{
    if (channel_->isWriting())
    {
        int savedErrno=0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(),&savedErrno);
        
        if (n > 0)
        {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes()==0)
            {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                {
                    //将回调函数加入到vector容器中，然后唤醒loop对应的线程去执行回调函数
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_,shared_from_this())
                    );

                    if (state_ == kDisconnecting)
                    {
                        shutdownInLoop();
                    }
                }
            }
            else
            {
                LOG_ERROR("TcpConnection::handleWrite");
            }
        }
        
    }
    else
    {
        LOG_ERROR("TcpConnection fd = %d is down, no more writing \n",channel_->fd());
    }
}

void TcpConnection::handleClose()
{
    LOG_INFO("cpConnection::handleClose fd=%d state=%d \n",channel_->fd(),state_.load());
    setState(kDisconnected);
    channel_->disableAll();

    TcpConnectionPtr connPtr = shared_from_this();
    
    connectionCallback_(connPtr);//执行连接关闭的回调
    closeCallback_(connPtr);//关闭连接后执行的回调
}

void TcpConnection::handleError()
{
    int optval;
    socklen_t optlen = sizeof(optval);
    int err = 0;
    if (::getsockopt(channel_->fd(),SOL_SOCKET,SO_ERROR,&optval,&optlen)<0)
    {
        err = errno;
    }
    else
    {
        err = optval;
    }

    LOG_ERROR("TcpConnection::handleError name:%s - SO_ERROR:%d \n",name_.c_str(),err);
    
}

void TcpConnection::sendInLoop()
{

}