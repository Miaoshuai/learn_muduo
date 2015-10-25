/*************************************************************************
    > File Name: Mutex.h
    > Author: MiaoShuai
    > Mail: 945970809@qq.com 
    > Created Time: 2015年10月24日 星期六 10时37分43秒
 ************************************************************************/

#ifndef MUDUO_BASE_MUTEX_H
#define MUDUO_BASE_MUTEX_H

#include<muduo/base/CurrentThread.h>
#include<booster/noncopyable.h>
#include<assert.h>
#include<pthread.h>
#include"CurrentThread.h"


#ifdef NDEBUG
__BEGIN_DECLS
extern void __assert_perror_fail(int errnum,const char *file,unsigned int line,const char *function)
	__THROW __attribute__ ((__noreturn__));
__END_DECLS
#endif

#define MCHECK(ret) ({	__typeof__ (ret) errnum = (ret);if(__builtin_expect(errnum != 0,0))	__assert_perror_fail(errnum,__FILE__,__LINE__,__func__;)})

#else //CHECK_PTHREAD_RETURN_VALUE

#define MCHECK(ret) ({__typeof__ (ret) errnum = (ret);assert(errnum == 0); (void) errnum;})

#endif //CHECK_PTHREAD_TETURN_VALUE

namespace muduo
{
	class MutexLock : booster::noncopyable
	{
		public:
			MutexLock():holder_(0)
			{
				MCHECK(pthread_mutex_init(&mutex_,NULL));
			}
			~MutexLock()
			{
				assert(holder_ == 0);
				MCHECK(pthread_mutex_destroy(&mutex_));

			}

			//判断是否被当前线程锁定
			bool isLockByThisThread()const
			{
				return holder_ == CurrentThread::tid();
			}

			void assertLocked()const
			{
				assert(isLockByThisThread());
			}

			//封装pthread_mutex_lock函数
			void lock()
			{
				MCHECK(pthread_mutex_lock(&mutex_));
				//获得锁拥有者
				 assignHolder();
			}

			//封装pthread_mutex_unlock
			void unlock()
			{
				unassignHolder();
				MCHECK(pthread_mutex_unlock(&mutex_));
			}

			//获取当前pthread_mutex_t结构体
			pthread_mutex_t * getPthreadMutex()
			{
				return &mutex_;
			}

		private:
			//Condition为其友元类
			friend class Condition;

			class UnassignGuard : booster::noncopyable
			{
				public:
					UnassignGuard(MutexLock & owner):owner_(owner)
					{
						owner_.unassignHolder();
					}	
					~UnassignGuard()
					{
						owner_.assignHolder();
				 	}
				private:
				 	MutexLock &owner_;
			};

			void unassignHolder()
			{
				holder_ = 0;
			}

			void assignHolder()
			{
				holder_ = CurrentThread::tid();
			}

			pthread_mutex_t mutex_;			//互斥锁
			pid_t			holder_;		//锁的拥有线程

	};

	//进一步封装锁，锁对象在其作用域结束时释放锁
	class MutexLockGuard : booster::noncopyable
	{
		public:
			explicit MutexLockGuard(MutexLock &mutex):mutex_(mutex)
			{
				mutex_.lock();
			}

			~MutexLockGuard()
			{
				mutex_.unlock();
			}
		private:
			MutexLock &mutex_;
	}
}

#define MutexLockGuard(x) error "Missing guard object name" 

#endif
