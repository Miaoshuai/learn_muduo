/*======================================================
    > File Name: EventLoop.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月04日 星期三 22时03分07秒
 =======================================================*/

#include <muduo/net/EventLoop.h>

#include <muduo/base/Logging.h>
#include <muduo/base/Mutex.h>
#include <muduo/net/Channel.h>
#include <muduo/net/Poller.h>
#include <muduo/net/SocketsOps.h>
#include <muduo/net/TimerQueue.h>

#include <boost/bind.hpp>

#include <signal.h>
#include <sys/eventfd.h>

using namespace muduo;
using namespace muduo::net;

namespace
{
    __thread EventLoop *t_loopInThisThread = 0;

    const int kPollTimeMs = 10000;

    int createEventfd()
    {
        int evtfd = ::eventfd(0,EFD_NONBLOCK | EFD_CFD_CLOEXEC);

        if(evtfd < 0)
        {
            LOG_SYSERR << "Failed in eventfd";
            abort();
        }
        return evtfd;
    }
}


EventLoop *EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

EventLoop::EventLoop()
    :looping_(false),
    quit_(false),
    eventHandling_(false),
    callingPendingFunctors_(false),
    iteration_(0),
    threadId_(CurrentThread::tid()),
    poller_(Poller::newDefaultPoller(this)),
    timerQueue_(new TimerQueue(this)),
    wakeupFd_(createEventfd()),
    wakeupChannel_(new Channel(this,wakeupFd_)),
    currentActiveChannel_(NULL)
{
    LOG_DEBUG << "EventLoop created " << this << "in thread" << threadId_;
    if(t_loopInThisThread)
    {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread << "exists in this thread" << threadId_;
    }

    else
    {
        t_loopInThisThread = this;
    }

    //注册读完成时的回调函数
    wakeupChannel_->setReadCallback(boost::bind(&EventLoop::handleRead,this));

    //注册可读事件
    wakeupChannel_->enableReading();
}


EventLoop::~EventLoop()
{
    LOG_DEBUG << "EventLoop" <<this<<"of thread" << threadId_
              <<"destructs in thread" << CurrentThread::tid();
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    assert(!looping_);
    assertInLoopThread();
    looping_ = true;
    quit_ = false;

    LOG_TRACE << "EventLoop" << this << "start looping";

    while(!quit_)
    {
        activeChannels_.clear();
        pollReturnTime_ = poller_->poll(kPollTimeMs,&activeChannels_);
        //计算执行多少次I/O返回
        if(Logger::LogLevel() <= Logger::TRACE)
        {
            printActiveChannels();
        }

        eventHandling_ = true;
        
        //处理就绪事件
        for(ChannelList::iterator it = activeChannels_.begin();
                it != activeChannels_.end(); ++it)
        {
            currentActiveChannel_ = *it;
            currentActiveChannel_->handleEvent(pollReturnTime_);
        }

        currentActiveChannel_ = NULL;
        eventHandling_ = false;
        //处理一些其他任务
        doPendingFunctors();
    }

    LOG_TRACE << "EventLoop" << this << "stop looping";
    looping_ = false;
}

void EventLoop::quit()
{
    quit_ = true;

    if(!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(const Functor &cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
}


void EventLoop::queueInLoop(const Functor &cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.push_back(cb);
    }

    if(!isInLoopThread() || callingPendingFunctors_)
    {
        wakeup();
    }
}


TimerId EventLoop::runAt(const Timestamp &time,const TimerCallback &cb)
{
    return timerQueue_->addTimer(cb,time,0.0);
}

TimerId EventLoop::runAfter(double delay,const TimerCallback &cb)
{
    Timestamp time(addTime(Timestamp::now(),delay));
    return runAt(time,cb);
}

TimerId EventLoop::runEvery(double interval,const TimerCallback &cb)
{
    Timestamp time(addTime(Timestamp::now()),interval);
    return timerQueue_->addTimer(cb,time,interval);
}


void  EventLoop::cancel(TimerId timerId)
{
    return timerQueue_->cancel(timerId);
}


void EventLoop::updateChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    poller_->updateChannel(channel);
}


void EventLoop::removeChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    if(eventHandling_)
    {
        assert(currentActiveChannel_ == channel ||
                std::find(activeChannels_.begin(),activeChannels_.end(),channel) == activeChannels_.end());
    }

    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop" << this 
              << "was created in threadId_ = " << threadId_
              << ",current thread id = " << CurrentThread::tid();
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = sockets::write(wakeupFd_,&one,sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR << "EventLoop::wakeup() writes" << n << "bytes instead of 8";
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = sockets::read(wakeupFd_,&one,sizeof one);
    if(n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() reads" << n << "bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> Functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        Functors.swap(pendingFunctors_);    //提高效率且防止死锁
    }

    for(ssize_t i = 0;i < Functors.size();++i)
    {
        Functors[i]();
    }
    callingPendingFunctors_ = false;
}


























































































































