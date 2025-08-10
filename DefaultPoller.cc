#include "Poller.h"
#include <stdlib.h>
#include "EPollPoller.h"

Poller* Poller::newDefaultPoller(EventLoop* loop){
    if(getenv("MUDUO_USE_POLL")){

        //生成poll实例
        return nullptr;
    }else{
        //默认生成epoll实例
        //TODO ...
        return new EPollPoller(loop);

    }
}
