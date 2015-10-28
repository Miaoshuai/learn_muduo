/*======================================================
    > File Name: Timestamp.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年10月28日 星期三 17时36分05秒
 =======================================================*/

#ifndef MUDUO_BASE_TIMESTAMP_H
#define MUDUO_BASE_TIMESTAMP_H

#include "copyable.h"
#include "Types.h"

#include <boost/operators.hpp>

namespace muduo
{
    //精度为微妙
    class Timestamp : public muduo::copyable,
                      public boost::less_than_comparable<Timestamp>
    {
        public:
             //无效构造函数
             Timestamp()
                 :microSecondsSinceEpoch_(0)
             {
             
             }

             //给定一个时间值的构造
             explicit Timestamp(int64_t microSecondsSinceEpoch)
                 :microSecondsSinceEpoch_(microSecondsSinceEpoch)
             {
             
             }

             //交换俩个时间戳
             void swap(Timestamp &that)
             {
                std::swap(microSecondsSinceEpoch_,that.microSecondsSinceEpoch_);
             }

             //字符串化时间戳
             string toString()const;

             //转化成一定格式的字符串
             string toFormattedString(bool showMicroseconds = true)const;

             //判断时间戳是否有效
             bool valid()const
             {
                return microSecondsSinceEpoch_ > 0;
             }

             //以int64_t类型返回当前时间戳
             int64_t microSecondsSinceEpoch()const
             {
                return microSecondsSinceEpoch_;
             }
             
             //以time_t为单位返回当前时间戳
             time_t secondsSinceEpoch()const
             {
                return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);  
             }

             //获得当前时间
             static Timestamp now();

             //获得当前无效时间
             static Timestamp invalid()
             {
                return Timestamp;
             }

             //获取unix时间
             Timestamp fromUnixTime(time_t t)
             {
                return fromUnixTime(t,0);
             }

             static const int kMicroSecondsPerSecond = 1000*1000;
        pricate:
            int64_t microSecondsSinceEpoch_;
    };

    //为Timestamp重载小于运算符
    inline bool operator<(Timestamp lhs,Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    
    }
    
    //重载==运算符
    inline bool operator==(Timestamp lhs,Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }   

    //俩个时间戳的时间差
    inline double timeDifference(Timestamp high,Timestamp low)
    {
        int64_t diff  = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff);
    }

    //俩个时间的总和
    inline Timestamp addTime(Timestamp timestamp,double seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
        return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
    }
}

#endif

















