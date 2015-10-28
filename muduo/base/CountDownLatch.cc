/*======================================================
    > File Name: CountDownLatch.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年10月28日 星期三 14时03分51秒
 =======================================================*/

#include "CountDownLatch.h"

using namespace muduo;

CountDownLatch::CountDownLatch(int count)
    :mutex_(),
    condition_(mutex_),
    count_(count)
{

}

//查看其他线程是否做完工作了，如果有一个没做完，那么它就还得继续等
void CountDownLatch::wait()
{
    MutexLockGuard lock(mutex_);
    while(count_ > 0)
    {
        condition_.wait();
    }
}
//被人家等待的线程执行做完工作后执行这个
void CountDownLatch::countDown()
{
    MutexLockGuard lock(mutex_);
    --count_;
    if(count_ == 0)
    {
        condition_.notifyAll();       
    }
}


int CountDownLatch::getCount()const
{
    MutexLockGuard lock(mutex_);
    return count_;
}






















