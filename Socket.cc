#include "Socket.h"
#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/tcp.h>

#include "InetAddress.h"
#include "Logger.h"


Socket::~Socket(){
    ::close(sockfd_);
}


void Socket::bindAddress(const InetAddress& localAddr){
    int ret = ::bind(sockfd_,(sockaddr*)localAddr.getSockAddr(),sizeof(sockaddr_in));
    if (ret!=0)
    {
        LOG_FATAL("bind sockfd:%d fail \n",sockfd_);
        return;
    }
    
}

void Socket::listen(){

    int ret = ::listen(sockfd_,1024);
    if (ret!=0)
    {
        LOG_FATAL("listen sockfd:%d fail \n",sockfd_);
        return;
    }
    
}


int Socket::accept(InetAddress* peeraddr){
    sockaddr_in addr;
    socklen_t len;
    bzero(&addr,sizeof(addr));
    int connfd = ::accept(sockfd_,(sockaddr*)&addr,&len);
    if (connfd>=0)
    {
        peeraddr->setSockAddr(addr);
    }
    
    return connfd;
}

void Socket::shutdownWrite(){
    int ret = ::shutdown(sockfd_,SHUT_WR);
    if(ret<0){
        LOG_ERROR("sockets::shutdownWrite eeror");
    }
}

void Socket::setTcpNoDelay(bool on){
    int optval = on?1:0;
    ::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&optval,sizeof(optval));
}

void Socket::setReuseAddr(bool on){
    int optval = on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof(optval));
}

void Socket::setReusePort(bool on){
    int optval = on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEPORT,&optval,sizeof(optval));
}

void Socket::setKeepAlive(bool on){
    int optval = on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_KEEPALIVE,&optval,sizeof(optval));
}