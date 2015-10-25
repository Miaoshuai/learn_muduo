/*************************************************************************
    > File Name: Condition.h
    > Author: MiaoShuai
    > Mail: 945970809@qq.com 
    > Created Time: 2015年10月24日 星期六 14时06分33秒
 ************************************************************************/

#ifndef MUDUO_BASE_CONDITION_H
#define MUDUO_BASE_CONDITION_H

#include<muduo/base/Mutex.h>
#include"Mutex.h"

#include<booster/noncopyable.h>
#include<pthread.h>

namespace muduo
{
 class Condition : booster::noncopyable
 {
	public:
		explicit Condition(MutexLock &mutex):mutex_(mutex)
		{
			MCHECK(pthread_cond_init(&pcond_,NULL));
		}
		
		~Condition()
		{
			MCHECK(pthread_cond_destroy(&pcond_));
		}

		//封装pthread_cond_wait
		void wait()
		{
			//由于接下来处于阻塞所以将锁的holder让出，当wait返回时，对象销毁，责锁的holder又是该线程
			MutexLock::UnassignGuard ug(mutex_);
			MCHECK(pthread_cond_wait(&pcond_,mutex_.getPthreadMutex()));
		}

		//wait阻塞时间
		bool waitForSeconds(int seconds);

		//通知某个线程等待结束
		void notify()
		{
			MCHECK(pthread_cond_signal(&pcond_));
		}

		//通知所有线程结束等待
		void notifyAll()
		{
			MCHECK(pthread_cond_broadcast(&pcond_));
		}
	private:
		MutexLock &mutex_;
		pthread_cond_t pcond_;
 };
}

#endif
