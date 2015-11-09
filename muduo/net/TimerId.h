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

        friend class TimerQueue;        //TimerQueue为其友元，可以操作其私有数据
        private:
            Timer *timer_;               //定时器指针
            int64_t sequence_;          //定时器序列号
    };
}
}
