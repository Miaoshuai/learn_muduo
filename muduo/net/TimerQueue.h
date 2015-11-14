/*======================================================
    > File Name: TimerQueue.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月09日 星期一 22时39分10秒
 =======================================================*/

#ifndef MUDUO_NET_TIMERQUEUE_H
#define MUDUO_NET_TIMERQUEUE_H

#include <set>
#include <vector>
#include <boost/noncoyable.hpp>
#include <muduo/base/Mutex.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Callback.h>
#include <muduo/net/Channel.h>

namespace muduo
{
namespace net
{
    class EventLoop;
    class Timer;
    class TimerId;

    class TimerQueue : boost::noncopyable
    {
        public:
            TimerQueue(EventLoop *loop);
            ~TimerQueue();


            TimerId addTimer(const TimerCallback &cb,
                            Timestamp when,
                            double interval);


            void cancel(TimerId timerId);
        
        private:
            typedef std::pair<Timestamp,Timer *> Entry;     //定时器入口
            typedef sta::set<Entry> TimerList;              //定时器集合
            typedef std::pair<Timer *,int64_t> ActiveTimer; //到时的定时器
            typedef std::set<ActiveTimeri> ActiveTimerSet;  //活跃的定时器集合

            void addTimerInLoop(Timer *timer);              //新增定时器
            void cancelInLoop(TimerId timerId);             //删除定时器集合

            //处理定时器超时
            void handleRead();

            //获得以超时的所有定时器
            std::vector<Entry> getExpired(Timestamp now);

            //如果定时器重复的话得重设定时器
            void reset(const std::vector<Entry> &expired,Timestamp now);

            //将定时器插入集合
            bool insert(Timer *timer);

            EventLoop *loop_;       //定时器所在的I/O循环
            const int timerfd_;     //定时器描述符
            Channel timerfdChannel_;

            TimerList timers_;      //定时器集合

            //for cancel()
            ActiveTimerSet activeTimers_;   //到期的定时器
            bool callingExpiredTimers_;
            ActiveTimerSet cancelingTimers_;


    }
}
}
