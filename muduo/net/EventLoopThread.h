/*======================================================
    > File Name: EventLoopThread.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月17日 星期二 17时33分43秒
 =======================================================*/

#ifndef MUDUO_NET_EVENTLOOPTHREAD_H
#define MUDUO_NET_EVENTLOOPTHREAD_H

#include <muduo/base/Condition.h>
#include <muduo/base/Mutex.h>
#include <muduo/base/Thread.h>

#include <boost/noncopyable.hpp>

namespace muduo
{
namespace net
{
    class EventLoop;

    class EventLoopThread : boost::noncopyable
    {
        public:
            typedef boost::function<void(EventLoop *)> ThreadInitCallback;
            
            EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(),
                            const string &name = string());
            ~EventLoopThread();
            //开始线程循环
            EventLoop *startLoop();
        private:
            void threadFunc();

            EventLoop *loop_;
            bool exiting_;
            Thread thread_;
            MutexLock mutex_;
            Condition cond_;
            ThreadInitCallback callback_;
    };
}


#endif
}
