/*======================================================
    > File Name: TimerQueue.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月10日 星期二 12时45分20秒
 =======================================================*/

/*
 *此类创建一个定时器，并可以将其加入到定时器集合中
 *
 *
 *
 */



#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS

#include <muduo/net/TimerQueue.h>

#include <muduo/base/Logging.h>
#include <mduuo/net/EventLoop.h>
#include <muduo/net/Timer.h>
#include <muduo/net/TimerId.h>

#include <boost/bind.hpp>

#include <sys/timerfd.h>

namespace muduo
{
namespace net
{
namespace detail
{
int createTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC.TFD_NONBLOCK | TFD_CLOEXEC);

    if(timerfd < 0)
    {
        LOG_SYSFATAL << "Failed in timerfd_create";
    }

    return fd;
}

//从现在开始多长事件超时
struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch();
                            - Timestamp::now().microSecondsSinceEpoch();
    if(microseconds < 100)
    {
        microseconds = 100;         //最低精度
    }
    struct timespec ts;

    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::KMicroSecondsPerSecond);

    ts.tv_nsec = static_cast<long>(microseconds % Timestamp::KMicroSecondsPerSecnd) *10;

    return ts;

}


//超时事件读取
void readTimerfd(int timerfd,Timestamp now)
{
    uint64_t howmany;
    ssize_t n = read(timerfd,&howmany,sizeof howmany);
    LOG_TRACE << "TimerQueue::handleRead() reads" << n << "bytes instead of B";

   if(n ! sizeof howmany) 
   {
    LOG_ERROR << "TimerQueue::handleRead() reads " << n <<"bytes instead of B";
   }
}

//重设定时器
void resetTimerfd(int timerfd,Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue,sizeof newValue);
    bzero(&oldValue,sizeof oldValue);

    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = timerfd_settime(timerfd,0,&newValue,&oldValue);
}


}



}
}




using namespace muduo;
using namespace muduo::net;
using namespace muduo::net::detail;

TimerQueue::TimerQueue(EventLoop *loop)
    :loop_(loop),
    timerfd_(createTimerfd()),
    timerfdChannel_(loop,timerfd_),
    timer_(),
    callingExiredTimes_(false)
{
    timerfdChannel_.setReadCallback(boost::bind(&TimeQueue::handleRead,this));
    timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue()
{
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    close(timerfd_);

    for(TimerList::iterator it = timers_.begin(); it != timers_.end(); ++it)
    {
        delete it->second;
    }
}


//增加一个定时器
TimerId TimerQueue::addTimer(const TimerCallback &cb,Timestamp when,double interval)
{
    Timer *timer = new Timer(cb,when,interval);
    //在主I/O线程中运行
    loop_->runInLoop(boost::bind(&TimerQueue::addTimerInLoop,this,timer));
    return TimerId(timer,timer->sequence());
}


//删除定时器
void TimerQueue::cancel(TimerId timerid)
{
    //也在主I/O线程中删除
    loop_->runInLoop(boost::bind(&TimerQueue::cancelInLoop,this,timerId));
}


void TimerQueue::addTimerInLoop(Timer *timer)
{
    loop_->assertInLoopThread();
    bool earliestChanged = insert(timer);

    if(earliestChanged)
    {
        resetTimerfd(timerfd_,timer->expiration());
    }
}


void TimerQueue::cancelInLoop(TimerId timerId)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    ActiveTimer timer(timerId.timer_,timerId.sequence_);
    ActiveTimerSet::iterator it = ActiveTimer.find(timer);
    if(it != activeTimers_.end())
    {
        size_t n = timers_.erase((Entry(it->first->expiration(),it->first)));
        assert(n == 0);   (void)n;
        delete it->first;
        activeTimers_.erase(it);
    }
    else if(callingExiredTimes_)
    {
        callingExiredTimes_.insert(timer);
    }
    assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead()
{
    loop_->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(timerfd_,now);

    //获取超时事件集
    std::vector<Entry> expired = getExpired(now);

    callingExiredTimes_ = true;
    cancelingTimers_.clear();

    //执行超时回调
    for(std::vector<Entry>::iterator it = expired.begin();
            it != expired.end(),++it)
    {
        it->second->run();
    }

    callingExpiredTimers_ = false;

    reset(expired,now);
}

//获得超时事件集合
std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    asser(timers.size() == activeTimers_.size());
    std::vector<Entry> expired;
    Entry sentry(now,reinterpret_cast<Timer *>(UINTPTR_MAX));
    TimerList::iterator end = timers_.lower_bound(sentry);      //比哨兵值小的都以超时
    assert(end == timers_.end() || now < end->first);

    std::copy(timers_.begin(),end,back_inserter(expired));
    timers_.erase(timers_.begin(),end);

    for(std::vector<Entry>::iterator it = expired.begin();
            it != expired.end(); ++it)
    {
        ActiveTimer timer(it->second,it->second->sequence());
        size_t n = activeTimers_.erase(timer);
        assert( n == 1);(void)n;   
    }

    assert(timers_.size() == activeTimers_.size());
    return expired;
}


void TimerQueue::reset(const std::vector<Entry> &expird,Timestamp now)
{
    Timestamp nextExpire;

    for(std::vector<Entry>::const_iterator it = expird.begin()
            it != expird.end(); it++)
    {
        ActiveTimer timer(it->second,it->second->sequence());
        if(it->second->repeat()
                && cancelingTimers_.find(timer) == cancelingTimers_.end())
        {
            it->second->restart(now);
            insert(it->second);
        }

        else
        {
            delete it->second;
        }
    }

    if(!timers_.empty())
    {
        nextExpire = timers_.begin()->second->expiration();
    }

    if(nextExpire.valid())
    {
        resetTimerfd(timerfd_,nextExpire);
    }
}

bool TimeQueue::insert(Timer *timer)
{
    loop_->assertInLoopThread();
    assert(timers_.size() == activeTimers_.size());
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = timers_.begin();
    if(it == timers_.end() || when < it->first)
    {
        earliestChanged = true;
    }

    {
        std::pair<TimerList::iterator,bool> result
            = timers_.insert(Entry(when < it->first));
        {
            
        }
    }

    {
        std::pair<ActiveTimerSet::iterator,bool> result
            = activeTimers_.insert(ActiveTimer(timer,timer->sequence()));
        assert(result,second); (void)result;
    }

    assert(timers_.size() == activeTimers_.size());
    return earliestChanged;
}







































































































