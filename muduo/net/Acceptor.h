/*======================================================
    > File Name: Accept.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月15日 星期日 13时41分19秒
 =======================================================*/

#ifndef MUDUO_NET_ACCEPTOR_H
#define MUDUO_NET_ACCEPTOR_H

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>

#include <muduo/net/Channel.h>
#include <muduo/net/Socket.h>

namespace muduo
{
namespace net
{
    class EventLoop;
    class InetAddress;

    class Acceptor : boost::noncopyable
    {
        public:
            typedef boost::function<void(int sockfd,const InetAddress &)> NewConnectionCallback;

            Acceptor(EventLoop *loop, const InetAddress &listenAddr, reuseport);
            ~Acceptor();

            //设置新连接的回调
            void setNewConnectionCallback(const NewConnectionCallback &cb)
            {
                newConnectionCallback_ = cb;
            }

            //判断是否在监听
            bool listenning()const
            {
                return listenning_;
            }

            //开始监听
            void listen();
        private:
            void handleRead();

            EventLoop *loop_;           //所属的loop循环
            Socket acceptSocket_;       //接受socket
            Channel acceptChannel_;     //管理acceptSocket_的channel
            NewConnectionCallback newConnectionCallback_;   //处理新连接的回调，可以自定义
            bool listenning_;           //是否监听
            int idlefd_;

    };
}
}


#endif 
