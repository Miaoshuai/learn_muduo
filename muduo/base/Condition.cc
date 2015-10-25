#include<muduo/base/Condition.h>
#include"Condition.h"

#include<errno.h>



//return true if time out ,false otherwise.
bool muduo::Condition::waitForSeconds(int seconds)
{
	struct timespec abstime;
	clock_gettime(CLOCK_REALTIME,&abstime);
	abstime.tv_sec + = seconds;
	MutexLock::UnassignGuard ug(mutex_);
	return ETIMEOUT == pthread_cond_timeout(&pcond_,mutex_,getPthreadMutex(),&abstime);
}
