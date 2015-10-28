/*************************************************************************
    > File Name: ThreadPool.h
    > Author: MiaoShuai
    > Mail: 945970809@qq.com 
    > Created Time: 2015年10月26日 星期一 18时44分54秒
 ************************************************************************/

#ifndef MUDUO_BASE_THREADPOOL_H
#define MUDUO_BASE_THREADPOOL_H


#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>
#include <muduo/base/Types.h>
#include "Mutex.h"
#include "Condition.h"
#include "Thread.h"
#include "Types.h"

#include <booster/function.h>
#include <booster/noncopyable.h>

#include <vector>
#include <deque>
#include <functional>
namespace muduo
{
	class ThreadPool : booster::noncopyable
	{
		public:
			//typedef booster::function<void ()> Task;
			explicit ThreadPool(const string &nameArg = string("ThreadPool"));
			~ThreadPool();

			//设置任务队列的最大值
			void setMaxQueueSize(int maxSize)
			{
				maxQueueSize_ = maxSize;
			}

			//设置初始化回调
			void setThreadInitCallback(const Task &cb)
			{
				threadInitCallback_ = cb;
			}

			//开启线程池
			void start(int numThreads);

			//停止线程池
			void stop();

			//获取名字
			const string &name()const
			{
				return name_;
			}

			//获得当前任务数
			size_t queueSize()const;

			void run(const Task &f);

		private:

			typedef std::function<void ()> Task;
			bool isFull()const;
			void runInThread();
			Task take();

			mutable MutexLock mutex_;
			Condition notEmpty_;
			Condition notFull_;
			string name_;
			Task threadInitCallback_;
			
            std::vector<Task> threads_;
			std::deque<Task> queue_;
			size_t maxQueueSize_;
			bool running_;
	};
}
