#include <sys/eventfd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

//防止一个线程创建多个EventLoop
__thread EventLoop * t_loopInThisThread =nullptr;
//定义默认的Poller IO复用接口的超时时间
const int kPoollTimeMs = 10000;

int createEventfd(){
    int evtfd= ::eventfd(0 , EFD_CLOEXEC | EFD_NONBLOCK);
    if (evtfd<0)
    {
        LOG_FATAL("eventfd error:%d \n",errno);
    }
    
    return evtfd;
}

EventLoop::EventLoop()
:looping_(false),quit_(false),
callingPendingFunctors_(false),
threadId_(CurrentThread::tid()),
poller_(Poller::newDefaultPoller(this)),
wakeupFd_(createEventfd()),
wakeupChannel_(new Channel(this,wakeupFd_))
{
    LOG_DEBUG("EventLoop created %p in thread %d \n",this,threadId_);
    //保证一个线程只运行一个eventLoop
    if (t_loopInThisThread)
    {
        LOG_FATAL("Another EventLoop %p exist in thread %d \n",t_loopInThisThread,threadId_);
    }else{
        t_loopInThisThread = this;
    }
    //设置wakeupFd感兴趣的事件类型，以及发生事件后的回调操作
    wakeupChannel_ ->setReadCallBack(std::bind(&EventLoop::handleRead,this));
    //每一个eventloop都会监听wakeChannel的EPOLLIN读事件
    wakeupChannel_->enableReading();
    
}


//在当前loop执行zb
void EventLoop::runInLoop(Functor cb){
    if (isInLoopThread()) //在当前的loop线程中，执行cb
    {
        cb();
    }
    else//在非loop线程中执行cb，就需要唤醒loop所在线程 ，然后执行cb
    {
        queueInLoop(cb);
    }
}



//把cb放入队列中，唤醒loop所在的线程，执行cb
void EventLoop::queueInLoop(Functor cb){

    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(cb);

    }
    //唤醒相应的，需要执行上面回调操作的loop的线程
    if ( !isInLoopThread() || callingPendingFunctors_)//callingPendingFunctors_的意思是：当前loop正在执行回调，但loop又有了新的回调
    {
        wakeup();//唤醒loop所在线程
    }
    

}








void EventLoop::handleRead(){
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_,&one,sizeof(one));
    if(n!=sizeof(one)){
        LOG_ERROR("EventLoop::handleRead() reads %lu bytes instead of 8",n);
    }
}

EventLoop::~EventLoop(){
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = nullptr;
}
//开启事件循环
void EventLoop::loop(){
    looping_=true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping \n",this);
    while(!quit_){
        activeChannels_.clear();

        //监听2类fd  一种是client的fd，一种是wakeup的fd
        poller_->poll(kPoollTimeMs,&activeChannels_);
        for(Channel * channel:activeChannels_){
            //Poller监听哪些channel发生事件了，然后上报给EventLoop，通知channel处理相应的事件

            channel->handleEvent(pollReturnTime_);
        }
        /*
            执行当前EventLoop事件循环需要处理的回调操作
            mainloop事先注册一个回调cb （需要subloop来执行）， wakeup subloop后，执行下面的方法，执行之前mainloop注册的方法
            QUESTION 不能理解这个回调是干嘛的
        */
        doPendingFunctors();
    }
    looping_=false;
    LOG_INFO("EventLoop %p stop looping. \n",this);
}


//退出事件循环  1.loop在自己的线程中调用quit， 2.在非loop的线程中调用quit
void EventLoop::quit(){
    quit_=true;
    if(!isInLoopThread()){ //如果在其他线程中，调用quit 在一个subloop中，调用了mainLoop的quit
        wakeup();
    }
}



//EventLoop的方法-->调用了Poller的方法
void EventLoop::updateChannel(Channel * channel){
    poller_->updateChannel(channel);


}


void EventLoop::removeChannel(Channel * channel){
    poller_->removeChannel(channel);

}


bool EventLoop::hasChannel(Channel *channel){

    return poller_->hasChannel(channel);

}

//用来唤醒loop所在的线程的 向wakefd_写一个数据，wakeupChannel就发生读事件，当前loop线程就会被唤醒
void EventLoop::wakeup(){
    uint64_t one = 1;
    ssize_t n= write(wakeupFd_,&one,sizeof one);
    if (n!=sizeof one)
    {
        LOG_ERROR("EventLooop::wakeup() writes %lu bytes instead of 8",n);
    }
    
}

//执行回调
void EventLoop::doPendingFunctors(){
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }
    for(const Functor& functor:functors){
        functor();//执行当前loop需要执行的回调操作
    }
    callingPendingFunctors_ = false;
}