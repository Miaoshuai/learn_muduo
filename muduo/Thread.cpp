/*************************************************************************
    > File Name: Thread.cpp
    > Author: MiaoShuai
    > Mail: 945970809@qq.com 
    > Created Time: 2015年10月25日 星期日 08时28分59秒
 ************************************************************************/

#include <muduo/base/Thread.h>
#include <muduo/base/CurrentThread.h>
#include <muduo/base/Exception.h>
#include <muduo/base/Logging.h>

#include "Thread.h"
#include "CurrentThread.h"
#include "Exception.h"
#include "Logging.h"


#include <booster/system_error.h>
#include <booster/traits/is_base_of.h>
#include <booster/weak_ptr.h>
#include <booster/shared_ptr.h>


#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/unistd.h>

namespace muduo
{
	namespace CurrentThread
	{
		__thread int t_cachedTid = 0;
		__thread char t_tidString[32];
		__thread int t_tidStringLength = 6;
		__thread const char *t_threadName = "unknow";

		//是相同类型会返回true,否则返回false
		const bool sameType = booster::is_same<int,pid_t>::value;
		//有问题
		assert(sameType);
	}

	namespace detail
	{
		//获得线程ID
		pid_t gettid()
		{
			//有问题
			return static_cast<pid_t>(gettid());
		}
		
		//给新fork的进程重新处理其主线程
		void afterFork()
		{
			muduo::CurrentThread::t_cachedTid = 0;
			muduo::CurrentThread::t_threadName = "main";
			CurrentThread::tid();
		}

		class ThreadNameInitializer
		{
			public:
  				ThreadNameInitializer()
				{
  					muduo::CurrentThread::t_threadName = "main";
					CurrentThread::tid();
					//防止死锁发生
					pthread_atfork(NULL,NULL,&afterFork);
				}
		};

		ThreadNameInitializer init;

		struct ThreadDate
		{
			typedef muduo::Thread::ThreadFunc ThreadFunc;
			ThreadFunc func_;
			string name_;
		    booster::weak_ptr<pid_t> wkTid_;	

			ThreadDate(const ThreadFunc& func,
 					const string &name,
					const booster::shared_ptr<pid_t>& tid)
				:func_(func),
				name_(name),
				wkTid_(tid)
			{

			}

			void runInThread()
			{
 				pid_t tid = muduo::CurrentThread::tid();

				booster::shared_ptr<pid_t> ptid = wkTid_.lock();
				if(ptid)
				{
					*ptid = tid;
					ptid.reset();
				}

				muduo::CurrentThread::t_threadName = name_.empty ? "muduoThread" : name_.c_str();

				//把参数t_threadName作为调用进程的经常名
				::prctl(PR_SET_NAME,muduo::CurrentThread::t_threadName);

				try
				{
					func_();
					muduo::CurrentThread::t_threadName = "finished";
				}
				catch (const Exception& ex)
  				  {
     				muduo::CurrentThread::t_threadName = "crashed";
      				fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      				fprintf(stderr, "reason: %s\n", ex.what());
      				fprintf(stderr, "stack trace: %s\n", ex.stackTrace());
     			    abort();
   				 }
    			catch (const std::exception& ex)
    			{
      				muduo::CurrentThread::t_threadName = "crashed";
      				fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
     				fprintf(stderr, "reason: %s\n", ex.what());
     				abort();
   				 }
    			catch (...)
    			{
      				muduo::CurrentThread::t_threadName = "crashed";
      				fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
      				throw; // rethrow
    			}				
			}
		};
	
		void *startThread(void *obj)
		{
			ThreadDate *data = static_cast<ThreadDate *>(obj);
			data->runInThread();
			delete data;
			return NULL
		}	

	}
}


using namespace muduo;

void CurrentThread::cacheTid()
{
	if(t_cachedTid == 0)
	{
		t_cachedTid = detail::gettid();
		t_tidStringLength = snprintf(t_tidString,sizeof t_tidString,"%5d",t_cachedTid);
	}
}

bool CurrentThread::isMainThread()
{
	return tid() == ::getpid();
}

void CurrentThread::sleepUsec(int64_t usec)
{
	struct timespec ts = {0,0};

	//time_t 为long类型,kMicr...为1000*1000
	ts.tv_sec  = static_cast<time_t>(usec / Timestamp::kMicroSecondsPerSecond);
	ts.tv_nsec = static_cast<time_t>(usec % Timestamp::kMicroSecondsPerSecond * 1000);

	::nanosleep(&ts,NULL);
}

//不懂
AtomicInt32 Thread::numCreated_;

Thread::Thread(const ThreadFunc &func,const string &n)
	:started_(false),
	joined_(false),
	pthreadId_(0),
	tid_(new pid_t(0)),
	func_(func),
	name_(n)
{
	setDefaultName();
}

#ifdef __GXX_EXPERIMENTAL_CXX0X__
Thread::Thread(ThreadFunc && func,const string &n)
	:started_(false),
	joined_(false),
	pthreadId_(0),
	tid_(new pid_t(0)),
	func_(std::move(func)),
	name_(n)
{
	setDefaultName();
}

#endif

Thread::~Thread()
{
	if(started_ && !joined_)
	{
		pthread_detach(pthreadId_);
	}
}

void Thread::setDefaultName()
{
	int num = numCreated_.incrementAndGet();
	if(name_.empty())
	{
		char buf[32];
		snprintf(buf,sizeof buf,"Thread%d",num);
		name_ = buf;
	}
}

//启动线程
void Thread::start()
{
	assert(!started_);
	started_ = true;
	
	//什么时候释放new的空间？
	detail::ThreadDate *data = new detail::ThreadDate(func_,name_,tid_);
	
	if(pthread_create(&pthreadId_,NULL,&detail::startThread,data))
	{
		started_ = false;
		delete data;
		LOG_SYSFATAL << "failed in pthread_create";
	}
	
}

int Thread::join()
{
	assert(started_);
	assert(!joined_);
	joined_ = true;
	return pthread_join(pthreadId_,NULL);
}







