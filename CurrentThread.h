#pragma once
#include <sys/syscall.h>      /* Definition of SYS_* constants */
#include <unistd.h>

/*
    pthread_self()返回的是pthread_t类型的线程ID，这是POSIX线程库提供的线程标识符，只在进程内部有效
    syscall(SYS_gettid)返回的是pid_t类型的线程ID，这是内核级别的线程ID，在整个系统中都是唯一的

*/
namespace CurrentThread{
    extern __thread int t_cachedTid;
    //缓存tid，因为获取tid是系统调用老是从系统调用获取，需要在用户态和内核态切换
    void cacheTid();
    inline int tid(){
        //如果为0缓存一下tid
        if (__builtin_expect(t_cachedTid ==0,0))
        {
            cacheTid();
        }
        return t_cachedTid;
    }
}