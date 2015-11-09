/*======================================================
    > File Name: Timer.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月09日 星期一 21时56分51秒
 =======================================================*/

#ifndef MUDUO_NET_TIMER_H
#define MUDUO_NET_TIMER_H

#include <boost/noncopyable.hpp>
#include <muduo/base/Atomic.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Callbacks.h>

namespace muduo
{
namespace net

    class Timer : boost::noncopyable
    {
        public:
            Timer(const TimerCallback &cb,Timestamp when,double interval)
                :callback_(cb),
                expiration_(when),
                interval_(interval),
                repeat_(interval > 0.0),
                sequence_(s_numCreated_.incrementAndGet())
        {}

            void run()const
            {
                callback_;
            }

            Timestamp expiration()const
            {
                return expiration_;
            }

            bool repeat()const
            {
                repeat_;
            }

            int64_t sequence()const
            {
                return sequence_;
            }

            void restart(Timestamp now);

            static int64_t numCreated()
            {
                return s_numCreated_.get();
            }
        private:
            const TimerCallback callback_;  //定时器回调函数
            Timestamp expiration_;          //定时器超时的时间值
            const double interval_;         //多次定时的周期
            const bool repeat_;             //是否周期定时
            const int64_t sequence_;        //周期定时的次数

            static AtomicInt64 s_numCreated_;

    };
}
