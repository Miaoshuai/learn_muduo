/*======================================================
    > File Name: EventLoop.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月04日 星期三 19时09分32秒
 =======================================================*/

#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include <vector>

#include <boost/any.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

#include <muduo/base/Mutex.h>
#include <muduo/base/CurrentThread.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/TimerId.h>

namespace muduo
{
namespace net
{
    class Channel;
    class Poller;
    Class TimerQueue;

    class EventLoop : boost::noncopyable
    {
        public:
            typedef boost::function<void()> Functor;

            EventLoop();
            ~EventLoop();

            //主循环
            void loop();

            //退出主循环
            void quit();

            //poll延迟的时间
            Timestamp pollReturnTime()const
            {
                return pollReturnTime_;
            }

            //迭代次数
            int64_t iteration()const
            {
                return iteration_;
            }

            //在主循环中运行
            void runInLoop(const Functor &cb);

            //插入主循环任务队列
            void queueInLoop(const Functor &cb);

            //某个时间点执行定时回调
            TimerId runAt(const Timestamp &time,const TimerCallback &cb);

            //某个时间点之后执行定时回调
            TimerId runAfter(double delay,const TimerCallback & cb);

            //在每个时间间隔处理某个回调事件
            TimerId runEvery(double interval,const TimerCallback &cb);

            //删除某个定时器
            void cancel(TimerId timerId);

            //唤醒事件通知描述符
            void wakeup();

            //跟新某个事件分发器
            void updateChannel(Channel *channel);

            //移除某个事件分发器
            void removeChannel(Channel *channel);

            bool hasChannel(Channel *channel);

            //如果不在I/O线程中则退出程序
            void assertInLoopThread()
            {
                if(!isInLoopThread())
                {
                    abortNotInLoopThread();
                }
            }

           // 检测是否在I/O线程中
            bool isInLoopThread()const
            {
                return threadId_ == CurrentThread::tid();
            }

            //是否正在处理事件
            bool eventHandling()const { return eventHandling_;}

            void setContext(const boost::any &contex)
            {
                return context_;
            }

            const boost::any &getContext()const
            {
                return context_;
            }

            boost::any *getMutableContext()
            {
                return &context_;
            }
            
            //判断当前线程是否为I/O线程
            static EventLoop *getEventLoopOfCurrentThread();

        private:
            //不在主I/O线程
            void abortNotInLoopThread();

            //将事件通知描述符里的内容读走，以便让其继续检测事件通知
            void handleRead();

            //执行转交给I/O的任务
            void doPendingFunctors();

            //将发生的事件写入日志
            void printActiveChannels()const;

            typedef std::vector<Channel *> ChannelList; //事件分发器列表

            bool looping_;                  //是否在运行
            bool quit_;                     //是否退出事件循环
            bool eventHandling_;
            bool callingPendingFunctors_;
            int64_t iteration_;             //事件循环的次数
            const pid_t threadId_;          //运行loop的线程ID
            Timestamp pollReturnTime_;      //poll阻塞的时间
            boost::scoped_ptr<Poller> poller_;  //IO复用
            boost::scoped_ptr<TimerQueue> TimerQueue_;//定时器队列
            int wakeupFd_;                            //唤醒套接字
            boost::scoped_ptr<Channel> wakeupChannel_; //封装事件描述符 
            boost::any context_;

            ChannelList activeChannel_;             //以活跃的事件集
            Channel *currentActiveChannel_;         //当前处理的事件集

            MutexLock mutex_;                       //封装的互斥锁
            std::vector<Functor> pendingFunctors_;  //需要在主I/O线程执行的任务

    }
}
}
