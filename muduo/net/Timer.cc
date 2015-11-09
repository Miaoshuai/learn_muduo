/*======================================================
    > File Name: Timer.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月09日 星期一 22时14分57秒
 =======================================================*/

#include <Timer.h>

using namespqce muduo;
using namespqce muduo::net;

AtomicInt64 Timer::s_numCreated_;

void Timer::restart(Timestamp now)
{
    if(repeat_)
    {
        expiration_ = addTime(now,interval_);           //将now和时间间隔interval_相加,获得下次超时时间
    }

    else
    {
        expiration_ = Timestamp::invalid();
    }
}
