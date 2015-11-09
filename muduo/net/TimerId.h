/*======================================================
    > File Name: TimerId.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月09日 星期一 21时40分12秒
 =======================================================*/

#ifndef MUDUO_NET_TIMERID_H
#define MUDUO_NET_TIMERID_H

#include <muduo/base/copyable.h>

namespace muduo
{
namespace net
{
    class TimerId : public muduo::copyable
    {
        public:
            TimerId()
                :timer_(NULL),
                sequence_(0)
        {
        
        }

        TimerId(Timer *timer,int64_t seq)
            :timer_(timer),
            sequence_(seq)
        {
        
        }

        friend class TimerQueue;
        private:
            Timer *timer_;
            int64_t sequence_;
    };
}
}
