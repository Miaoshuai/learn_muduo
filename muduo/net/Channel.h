/*======================================================
    > File Name: Channel.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月04日 星期三 15时24分20秒
 =======================================================*/

#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <muudo/base/Timestamp.h>

namespace muduo
{
namespace net
{
    class EventLoop;
    
    class Channel : boost::noncopyable
    {
        public:
            typedef boost::function<void()> EventCallback;
            typedef boost::function<void(Timestamp)> ReadEventCallback;
            
            Channel(EventLoop *loop,int fd);
            ~Channel();

            //处理事件
            void handleEvent(Timestamp receiveTime);

            //设置可读事件回调
            void setReadCallback(const ReadEventCallback &cb)
            {
                readCallback_ = cb;
            }

            //设置可写事件回调
            void setWriteCallback(const EventCallback &cb)
            {
                writeCallback_ = cb;
            }

            //设置关闭事件回调
            void setCloseCallback(const EventCallback &cb)
            {
                closeCallback_ = cb;
            }

            //设置错误事件回调
            void setErrorCallback(const EventCallback &cb)
            {
                errorCallback_ = cb;
            }

            void tie(const boost::shared_ptr<void>&);

            //返回当前Channel服务的fd
            int fd()const
            {
                return fd_;
            }

            //返回注册的事件
            int events()const
            {
                return events_;
            }

            //设置注册事件
            void set_revents(int revt)
            {
                revents_ = revt;
            }

            //判断是否注册的事件
            bool isNoneEvent()const
            {
                return events_ == kNoneEvent;
            }

            //设置可读事件
            void enableReading()
            {
                events_ |= kReadEvent;
                update();
            }

            //销毁读事件
            void disableReading()
            {
                events_ &= ~kReadEvent;
                update();
            }

            //注册写事件
            void enableWriting()
            {
                events_ |= kWriteEvent;
             update();   
            }

            //销毁写事件
            void disableWriting()
            {
                events_ &= ~kWriteEvent;
                update();
            }

            //销毁所有事件
            void disableAll()
            {
                events_ = kNoneEvent;
                update();
            }

            //是否注册可写事件
            isWriting()const
            {
                return events_ & kWriteEvent;
            }

            //for Poller
            int index()
            {
                return index_;
            }

            void set_index(int dex)
            {
                index_ = dex;
            }

            //for debug
            string reventsToString()const;
            string eventsToString()const;

            void doNotLogHup()
            {
                logHup_ = false;
            }

            //该channel所属的loop
            EventLoop *ownerLoop()
            {
                return loop_;
            }

            //移除该事件分发器
            void remove();
        private:
            //将事件字符串化
            static string eventsToString(int fd,int ev);

            //更新事件事件分发器
            void update();

            //事件处理
            void handleEventWithGuard(Timestamp receiveTime);

            //定义事件类型变量
            static const int kNoneEvent;    //无事件
            static const int kReadEvent;    //可读事件
            static const int kWriteEvent;   //可写事件

            EventLoop *loop_;               //channel所属的loop
            const int fd_;                  //channel负责的文件描述符
            int events_;            //注册的事件
            int revents_;           //就绪的事件
            int index_;             //被poller使用的下标
            bool logHup_;           //是否生成某些日志

            boost::weak_ptr<void> tie_; //
            bool tied_;
            bool eventHandling_;
            bool addedToLoop_;
            ReadEventCallback readCallback_;    //读事件回调
            EventCallback writeCallback_;       //写事件回调
            EventCallback closeCallback_;       //关闭事件回调
            ReadEventCallback errorCallback_;   //错误事件回调

    };
}
}

#endif
