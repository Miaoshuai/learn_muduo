/*************************************************************************
    > File Name: Thread.h
    > Author: MiaoShuai
    > Mail: 945970809@qq.com 
    > Created Time: 2015年10月24日 星期六 22时07分36秒
 ************************************************************************/

#ifndef MUDUO_BASE_THREAD_H
#define MUDUO_BASE_THREAD_H

#include<muduo/base/Atomic.h>
#include<muduo/base/Types.h>
#include"Atomic.h"
#include"Types.h"

#include<booster/function.h>
#include<booster/noncopyable.h>
#include<booster/shared_ptr.h>
#include<pthread.h>

namespace muduo
{
	class Thread : booster::noncopyable
	{
		public:
			//function类似函数指针功能，比函数指针应用范围广
			typedef booster::function<void ()> ThreadFunc;

			explicit Thread(const ThreadFunc&,const string &name = string());

			#ifdef __GXX_EXPERIMENTAL_CXX0X__
			explicit Thread(ThreadFunc &&,const string &name = string());
			#endif

			~Thread();

			//启动线程
			void start();

			//线程阻塞
			int join();

			//返回线程是否开始运行的状态
			bool started()const
			{
				return started_;
			}

			//返回线程ID
			pid_t tid()const
			{
				return *tid_;
			}

			//返回名字
			const string & name()const
			{
				return name_;
			}

			//创建线程的总量
			static int numCreated()
			{
				return numCreated_.get();
			}
			
		private:
			void setDefaultName();

			bool						started_;//是否启动
			bool						joined_;//是否阻塞
			pthread_t					pthreadId_;//线程ID结构体
			booster::shared_ptr<pid_t>  tid_;  //系统级线程ID
			ThreadFunc					func_;//线程函数
			string						name_;//线程名字

			static AtomicInt32			numCreated_;
	};
}

#endif
