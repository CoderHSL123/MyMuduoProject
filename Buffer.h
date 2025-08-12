#pragma once


#include <vector>
#include <string>

class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initialSize = kInitialSize)
        :buffer_(kCheapPrepend+kInitialSize),
        readerIndex_(kCheapPrepend),
        writerIndex_(kCheapPrepend)
        {}
    //Buffer中可读数据的长度
    size_t readableBytes() const
    {
        return writerIndex_ - readerIndex_;
    }

    size_t writableBytes() const
    {
        return buffer_.size() - writerIndex_;
    }

    size_t prependableBytes() const
    {
        return readerIndex_;
    }


    //buffer_.size() - writerIndex_ 
    void ensureWriteableBytes(size_t len)
    {
        if (writableBytes()<len)
        {
            makeSpace(len);//扩容函数
        }
        

    }
    //把[data,data+len]内存上的数据，添加到writable缓冲区当中
    void append(const char * data,size_t len)
    {
        ensureWriteableBytes(len);
        std::copy(data,data+len,beginWrite());
        writerIndex_ += len;
    }

    char* beginWrite()
    {
        return begin() + writerIndex_;
    }

    const char* beginWrite() const
    {
        return begin() + writerIndex_;
    }

    //从fd上读数据
    ssize_t readFd(int fd,int* saveErrno);

    //通过fd发送数据
    ssize_t writeFd(int fd, int* savedErrno);


        //返回缓冲区中可读数据的起始地址
    const char* peek() const{
        return begin() + readerIndex_;
    }

    //onMessage string <- Buffer
    void retrieve(size_t len)
    {
        if(len < readableBytes())
        {
            //执行此代码表明数据只是读取可读取部分的一部分数据
            readerIndex_ +=len;
        }
        else
        {
            retrieveAll();
        }
    }
    //此函数表示已经读完所有的数据，在读完所有数据后直接将两个index重新置位到kCheapPrepen
    void retrieveAll()
    {
        readerIndex_ = writerIndex_=kCheapPrepend;
    }
    //将onMessage函数上报的Buffer转化为string返回
    std::string retrieveAllString()
    {
        return retrieveAsString(readableBytes());
    }

    std::string retrieveAsString(size_t len)
    {
        std::string result(peek(),len);
        retrieve(len); //上行代码把缓冲区中可读的数据，已经读取出来，这里需要对缓冲区进行复位操作
        return result;
    }

private:

    void makeSpace(size_t len){
        //已经读取的数据部分 加上 剩余可写的部分 比len还小 就需要扩容   否则就把还没读的数据向前移动覆盖已经读取的地方
        if(writableBytes()+prependableBytes()-kCheapPrepend<len)
        {
            buffer_.resize(writerIndex_ +len);
        }
        else
        {
            //将数据向前移动覆盖已读取的数据，让后续数据 有空间可写
            size_t readable = readableBytes();//未读数据量
            std::copy(begin()+readerIndex_,
                      begin()+writerIndex_,
                      begin()+kCheapPrepend);
            readerIndex_=kCheapPrepend;
            writerIndex_= readerIndex_+readable;
        }
    }

    char *begin(){ return &(*buffer_.begin());}

    const char* begin() const { return &(*buffer_.begin()); }


    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

};