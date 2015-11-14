/*======================================================
    > File Name: Pooler.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月04日 星期三 14时48分28秒
 =======================================================*/

#ifndef MUDUO_NET_POLLER_H
#define MUDUO_NET_POLLER_H

#include <map>
#include <vector>
#include <boost/noncopyable.hpp>

#include <muduo/base/Timestamp.h>
#include <muduo/net/EventLoop.h>

namespace muduo
{
namespace net
{
    class Channel;

    class Poller : boost::noncopyable
    {
        public:
            typedef std::vector<Channel *> ChannelList;
            
            Poller(EventLoop *loop);

            virtual ~Poller();

            //不许在I/O线程中调用,I/O复用的封装
            virtual Timestamp poll(int timeoutMs,ChannelList *activeChannels) = 0;

            //跟新Channel
            virtual void updateChannel(Channel *channel) = 0;

            //移除Channel
            virtual void removeChannel(Channel *channel) = 0;

            //这个channel是否在map中存在
            virtual bool hasChannel(Channel *channel)const;

            //默认poller方式
            static Poller *newDefaultPoller(EventLoop *loop);

            void assertInLoopThread()const
            {
                ownerLoop_->assertInLoopThread();
            }

        protected:
            typedef std::map<int,Channel*> ChannelMap;
            ChannelMap Channels_;           //存储事件分发器的map
        private:
            EventLoop *owerLoop_;           //属于哪个loop
    };
}
}

#endif
