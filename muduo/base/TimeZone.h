/*======================================================
    > File Name: TimeZone.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年10月29日 星期四 08时18分21秒
 =======================================================*/

#ifndef MUDUO_BASE_TIMEZONE_H
#define MUDUO_BASE_TIMEZONE_H

#include "copyable.h"
#include <boost/shared_ptr.hpp>
#include <time.h>

namespace
{
    class Timezone : public muduo::copyable
    {
        public:
            explicit Timezone(const char *zonefile);
            Timezone(int eastofUtc,const char *tzname);  //一个固定的时间域
            Timezone(){}
            bool valid()const
            {
                return static_cast<bool>(data_);
            }           

            struct tm toLocalTime(time_t secondsSinceEpoch)const;

            time_t fromLocalTime(const struct tm&)const;

            static struct tm toUtcTime(const struct tm&);

            static time_t fromUtcTime(int year,int month,int day,int hour,int minute,int seconds);

            

            struct Data;
        private:
            boost::shared_ptr<Date> data_;
    };   
}

#endif
