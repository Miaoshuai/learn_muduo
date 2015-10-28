#include <muduo/base/ThreadPool.h>
#include "ThreadPool.h"
#include "Mutex.h"

#include <muduo/base/Exception.h>

#include <booster/function.h>
#include <functional>


#include <assert.h>
#include <stdio.h>
#include <iostream>

using namespace muduo;

TihreadPool::ThreadPool(const string &nameArg)
	:mutex_(),
	notEmpty_(mutex_),
	notFull_(mutex_),
	name_(nameArg),
	maxQueueSize_(0),
	running_(false)
{
    	
}

ThreadPool::~ThreadPool()
{
	if(running_)
	{
        stop();		
	}
}


//启动线程池
void ThreadPool::start(int numThreads)
{
	assert(threads_.empty());
	runing_ = true;
	threads_.reserve(numThreads);

	for(int i = 0;i < numThreads; i++)
	{
		char id[32];
		//将该线程ID字符串化
		snprintf(id,sizeof id,"%d",i+1);
        threads_.push_back(new Thread(std::bind(&ThreadPool::runInThread,this),name_ + id));
        //每个线程都执行刚由bind绑定后的函数
        threads_[i].start();
    }

}

void ThreadPool::stop()
{
    {
        MutexLockGuard lock(mutex_);
        runing_ = false;
        notEmpty_.notifyAll();
    }
    //主线程阻塞等待子线程结束   
    foreach(threads_.begin(),threads_.end(),std::bind(&muduo::Thread::join,_1));
}


//任务队列的长度
size_t ThreadPool::queueSize()const
{
    MutexLockGuard lock(Mutex_);
    return queue_.size();
}


//往任务队列添加任务
void ThreadPool::run(const Task &task)
{
    if(threads_.empty())
    {
        task();
    }
    else
    {
        MutexLockGuard lock(Mutex_);
        while(isFull())
        {
            notFull_.wait();
        }
        assert(!isFull());

        queue_.push_back(task);
        notEmpty_.notify();
    }
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
void ThreadPool::run(Task &&task)
{
    if(threads_.empty())
    {
        task();        
    }
    else
    {
        MutexLockGuard lock(Mutex_); 
        while(isFull)
        {
            notFull_.wait();
        }
        assert(!isFull);
        queue_.push_back(std::move(task));
        notEmpty_.notify();      
    }
}

#endif

//取任务
ThreadPool::Task ThreadPool::take()
{
    MutexLockGuard lock(Mutex_);

    //任务队列为空时它就会等待
    while(queue_.empty() && runing_)
    {
        notEmpty_.wait();
    }

    Task task;
    if(!queue_.empty())
    {
        task = queue_.front();
        queue_.pop_front();
        if(maxQueueSize_ > 0)
        {
            notFull_.notify();
        }

        return task;
    }
}


bool ThreadPool::isFull()const
{
    mutex_.assertLocked();
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

//线程运行任务队列的任务
void ThreadPool::runInThread()
{
    try
    {
        if(threadInitCallback_)
        {
            threadInitCallback_();
        }
        while(runing_)
        {
            Task task(take());
            if(task)
            {
                task();
            }
        }
    }
     catch (const Exception& ex)
           {
                   fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
                       fprintf(stderr, "reason: %s\n", ex.what());
                           fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
                               abort();
                                 }
       catch (const std::exception& ex)
             {
                     fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
                         fprintf(stderr, "reason: %s\n", ex.what());
                             abort();
                               }
         catch (...)
               {
                       fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
                           throw; // rethrow
                             }

}



























