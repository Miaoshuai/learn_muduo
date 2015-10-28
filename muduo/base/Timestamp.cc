/*======================================================
    > File Name: Timestamp.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年10月28日 星期三 19时07分22秒
 =======================================================*/

#include "Timestamp.h"

#include <sys/time.h>
#include <stdio.h>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <inttypes.h>

#include <boost/static_assert.hpp>

using namespace muduo;

BOOST_STATIC_ASSERT(sizeof(Timestamp) == sizeof(int64_t));

string Timestamp::toString()const
{
    char buf[32] = {0};
    int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    int64_t microseconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
    //格式化
    snprintf(buf,sizeof(buf)-1,"%" PRId64".%06" PRId64"",seconds,microseconds);
    return buf;
}

string Timestamp::toFormattedString(bool showMicroseconds)const
{
    char buf[32] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    struct tm tm_time;
    //将日期和时间转化为格林威治(GMT)的时间函数,结果存储在tm_time中
    gmtime_r(&seconds,&tm_time);

    //如果要显示微妙
    if(showMicroseconds)
    {
        int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
        snprintf(buf,sizeof(buf),"%4d%02d%02d %02d:%02d:%02d.%06d",
                tm_time.tm_year + 1900,tm_time.tm_mon + 1,tm_time.tm_mday,
                tm_time.tm_hour,tm_time.tm_min,tm_time.tm_sec,
                microseconds);
    }
    else
    {
                
        snprintf(buf,sizeof(buf),"%4d%02d%02d %02d:%02d:%02d",
                tm_time.tm_year + 1900,tm_time.tm_mon + 1,tm_time.tm_mday,
                tm_time.tm_year + 1900,tm_time.tm_mon + 1,tm_time.tm_mday,
                tm_time.tm_hour,tm_time.tm_min,tm_time.tm_sec);
    }
    return buf;   
}


Timestamp Timestamp::now()
{
    struct timeval tv;
    //获得微秒级的时间
    gettimeofday(&tv,NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}


























