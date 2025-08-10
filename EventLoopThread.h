#pragma once
#include <functional>


#include "noncopyable.h"


class EventLoop;

class EventLoopThread: Noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

private:
    EventLoop* loop_;
    bool exiting_;


};

