/*======================================================
    > File Name: BoundedBlockingQueue.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年10月28日 星期三 14时57分41秒
 =======================================================*/


/*
    阻塞的任务队列
 *
 */


#ifndef MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H
#define MUDUO_BASE_BOUNDEDBLOCKINGQUEUE_H

#include "Condition.h"
#include "Mutex.h"

#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>
#include <assert.h>
#include <vector>

namespace muduo
{
    template<typename T>
    class Boundedblocking : boost::noncopyable
    {
        public:
            explicit Boundedblocking(int maxSize)
                :mutex_(),
                notEmpty_(mutex_),
                notFull_(mutex_),
                queue_(maxSize)
        {
        
        }
        //添加任务
        void put(const T &x)
        {
            MutexLockGuard lock(mutex_);
            while(queue_.full())
            {
                notFull_.wait();
            }
            assert(!queue_.full());
            queue_.push_back(x);
            notEmpty_.notify();
        }
        //取任务
        T take()
        {
            MutexLockGuard lock(mutex_);
            while(queue_.empty())
            {
                notEmpty_.wait();
            }
            assert(!queue_.empty());
            T front(queue_.front());
            queue_.pop_front();
            notFull_.notify();
            return front;
        }
        
        bool empty()const
        {
            MutexLockGuard lock(mutex_);
            return queue_.empty();
        }

        bool full()const
        {
            MutexLockGuard lock(mutex_);
            return queue_.full();
        }

        size_t size()const
        {
            MutexLockGuard lock(mutex_);
            return queue_.size();
        }

        size_t capacity()const
        {
            MutexLock lcok(mutex_);
            return queue_.capacity();
        }
        private:
            mutable MutexLock           mutex_;
            Condition                   notEmpty_;
            Condition                   notFull_;
            boost::circular_buffer<T>   queue_;     //类似与循环队列
                        
    }
}
