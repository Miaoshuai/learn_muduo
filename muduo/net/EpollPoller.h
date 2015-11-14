/*======================================================
    > File Name: EpollPoller.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月05日 星期四 15时12分42秒
 =======================================================*/

#ifndef MUDUO_NET_POLLER_EPOLLPOLLER_H
#define MUDUO_NET_POLLER_EPOLLPOLLER_H
#include <vector>
#include <muduo/net/Poller.h>

class EPollPoller : public Poller
{
    public:
        EPollPoller(EventLoop *loop);
        virtual Timestamp poll(int timeoutMs,ChannelList *activeChannels);   //内部调用epoll_wait函数

        virtual void updateChannel(Channel *channel);

        virtual void removeChannel(Channel *Channel);       
    private:
        static const int kInitEventListSize = 16;           //epoll事件表的大小

        static const char *operatoionToString(int op);

        void fillActiveChannels(int numEvents,ChannelList *activeChannels)const; //将epoll返回的活跃事件填充到activeChannel

        void update(int operation,Channel *channel);    //对Channel的更改操作

        typedef std::vector<struct epoll_event> EventList;

        int epollfd_;       //epoll的事件表fd

        EventList events_;  //epoll事件数组
};
