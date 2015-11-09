/*======================================================
    > File Name: EpollPoller.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月05日 星期四 15时30分55秒
 =======================================================*/

#include <muduo/net/poller/EPollPoller.h>

#include <muduo/base/Logging.h>
#include <muduo/net/Channel.h>

#include <boost/static_assert.hpp>

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>

using namespace muduo;
using namespace muduo::net;

BOOST_STATIC_ASSERT(EPOLLIN == POLLIN);
BOOST_STATIC_ASSERT(EPOLLPRI == POLLPRI);
BOOST_STATIC_ASSERT(EPOLLOUT == POLLOUT);
BOOST_STATIC_ASSERT(EPOLLRDHUP == POLLRDHUP);
BOOST_STATIC_ASSERT(EPOLLERR == POLLERR);
BOOST_STATIC_ASSERT(EPOLLHUP == POLLHUP);

namespace
{
    const int KNew = -1;
    const int KAdded = 1;
    const int KDeleted = 2;
}


::EPollPoller::EPollPoller(EventLoop * loop)
    :Poller_(loop),
    epollfd_(::epoll_create(::EPOLL_CLOEXEC)),
    events_(kInitEventListSize)
{
    if(epollfd_ < 0)
    {
        LOG_SYSFATAL << "EPollPoller::EPollPoller";
    }
}

EPollPoller::~EPollPoller()
{
    close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs,ChannelList *activeChannels)
{
    LOG_TRACE << "fd total count" << channels_.size();

    int numEvents = epoll_wait(epollfd_,
                               &*events_.begin(),
                               static_cast<int>(events_.size())
                               timeoutMs);

    int savedErrno = errno;
    Timestamp now(Timestamp::now());
    if(numEvents > 0)
    {
        LOG_TRACE << numEvents << "events happended";

        fillActiveChannels(numEvents,activeChannels);
        if(implicit_cast<size_t>(numEvents) == events_.size())
        {
            events_.resize(events_.size()*2);
        }
    }
    else if(numEvents == 0)
    {
        LOG_TRACE << "nothing happend";
    }
    else
    {
        if (savedErrno != EINIR)
        {
            errno = savedErrno;
            LOG_SYSFATAL << "EPollPoller::poll()";
        }
    }
    return now;
}


void EPollPoller::fillActiveChannels(int numEvents,ChannelList *activeChannels)const
{
    assert(implicit_cast<size_t>(numEvents) <= events_.size());

    for(int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel *>(events_.data.ptr);
#ifndef NDEBUG
        int fd = channel->fd();
        ChannelMap::const_iterator it = channel_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);
#endif
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
    }
}


void EPollPoller::updateChannel(Channel *channel)
{
    Poller::assertInLoopThread();
    const int index = channel->index();
    LOG_TRACE << "fd = " << channel->fd()
        << " events = " << channel->events() << " index = " << index;

    if(index == KNew || index == KDeleted)
    {
        int fd = channel->fd();
        if(index == KNew)
        {
            assert(channel_.find(fd) == channel_.end());
            channel_[fd] = channel;
        }

        else
        {
            assert(channels_.find(fd) != channels_.end());

            assert(channels_[fd] == channel_);
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);
    }
    else
    {
        int fd = channel->fd();
        (void)fd;
        assert(channel_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);

        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(KDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }
    }
}


void EPollPoller::removeChannel(Channel *channel)
{
    Poller::assertInLoopThread();

    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());

    int index = channel->index();
    assert(index == kAdded || index == KDeleted);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);

    if(index == kAdded)
    {
        update(EPOLL_CTL_DEL,channel);
    }
    channel->set_index(kNew);
}

void EPollPoller::update(int operation,Channel *channel)
{
    struct epoll_event event;
    bzero(&event,sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl op = " << operayionToString(operation)
        << "fd = " << fd << "event = {" << channel->reventsToString()<<"}";

    if(::epoll_ctl(epollfd_,operation,fd,&event) < 0)
    {
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_SYSERR << "epoll_ctl op = " << operayionToString(operation) << "fd = " << fd;
        }
        else
        {
            LOG_SYSFATAL << "epoll_ctl op = " << operayionToString(operation) << "fd = "<<fd;
        }
    }
}


const char *EPollPoller::operationToString(int op)
{
    switch (op)
    {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
            return "Unknown Operation";
    }
}































































































































































