/*************************************************************************
    > File Name: BlockingQueue.h
    > Author: MiaoShuai
    > Mail: 945970809@qq.com 
    > Created Time: 2015年10月24日 星期六 15时27分50秒
 ************************************************************************/
#ifndef MUDUO_BASE_BLOCKINGQUEUE_H
#define MUDUO_BASE_BLOCKINGQUEUE_H

#include<muduo/base/Condition.h>
#include<muduo/base/MutexLock.h>
#include"Condition.h"
#include<MutexLock.h>

#include<booster/noncopyable.h>
#include<deque>
#include<assert.h>

namespace muduo
{
	template<typename T>
	class BlockingQueue : booster::noncopyable
	{
		public:
			BlockingQueue()
				:mutex_(),
				notEmpty_(mutex_),
				queue_()
			{

			}

			//往任务队列中添加东西
			void put(const T& x)
			{
				MutexLockGuard lock(mutex_);
				queue_.push_back(x);
				//唤醒某个线程取任务
				notEmpty_.notify();
			}
		#ifdef __GXX_EXPERIMENTAL_CXX0X__
			
			//此处T为模板类型，所以，下面的引用既可以接收左值也可以接收右值
			void put(T &&x)
			{
				MutexLockGuard lock(mutex_);
				queue_.push_back(std::move(x));
				notEmpty_.notify();
			}
		#endif

			//取任务
			T take()
			{
				MutexLockGuard lock(_mutex);

				//循环等待任务队列不为空
				while(queue_.empty())
				{
					notEmpty_.wait();
				}

				assert(!queue_.empty());
				
				//取出任务队列的第一个元素

			#ifdef __GXX_EXPERIMENTAL_CXX0X__
				T front(std::move(queue_.front()));
			#else
				T front(queue_.front());
			#endif
				queue_.pop_front();
				return front;
			}

			size_t size()const
			{
				MutexLockGuard lock(mutex_);
				return queue_.size();
			}

		private:
			mutable MutexLock mutex_;
			Condition         notEmpty_;
			std::deque<T>	  queue_;
	}
}

#endif
