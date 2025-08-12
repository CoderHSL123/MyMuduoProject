#include "Buffer.h"
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
/*
    从fd上读数据 Poller工作在LT模式
    
*/
ssize_t Buffer::readFd(int fd,int* saveErrno)
{
    char extrabuf[65536]={0}; //栈上的内存空间
    
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin()+writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    const int iovcnt = (writable<sizeof(extrabuf))?2:1;
    const ssize_t n = ::readv(fd,vec,iovcnt);
    if(n<0){
        *saveErrno = errno;
    }else if(n<=writable){//Buffer的缓冲区能够存储读出来的数据
        writerIndex_+=n;
    }else{//Buffer的穿冲去不能完全装下读出来的数据，一部分数据写入extrabuf
        writerIndex_ = buffer_.size();
        //appen内部对buffer进行了扩容
        append(extrabuf,n-writable); //从writerIndex_开始写，记录n-writable大小的数据
    }
    return n;
}


ssize_t Buffer::writeFd(int fd, int* savedErrno){
    ssize_t n = ::write(fd,peek(),readableBytes());
    if(n<0)
    {
        *savedErrno = errno;
    }
    return n;
}