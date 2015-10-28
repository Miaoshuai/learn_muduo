/*======================================================
    > File Name: CountDownLatch.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年10月28日 星期三 13时36分23秒
 =======================================================*/


/*
 这个类的主要是同步的一个辅助类，它的功能是：在完成一组正在由其他线程执行的操作之前，它允许一个或多个线程一直等待
 
 */

#ifndef MUDUO_BASE_COUNTDOWNLATCH_H
#define MUDUO_BASE_COUNTDOWNLATCH_H

#include "Condition.h"
#include "Mutex.h"

#include <boost/noncopyable.hpp>

namespace muduo
{
    class CountDownLatch : boost::noncopyable
    {
        public:
            explicit CountDownLatch(int count);
            void wait();
            void countDown();
            int getCount()const;
        private:
            mutable MutexLock mutex_;
            Condition condition_;
            int count_;
    }
}

