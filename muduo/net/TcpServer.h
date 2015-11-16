/*======================================================
    > File Name: TcpServer.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月16日 星期一 22时09分21秒
 =======================================================*/

#ifndef MUDUO_NET_TCPSERVER_H
#define MUDUO_NET_TCPSERVER_H

#include <muduo/base/Atomic.h>
#include <muduo/base/Types.h>
#include <muduo/net/TcpConnection.h>

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace muduo
{
namespace net
{
    class Acceptor;
    class EventLoop;
    class EventLoopThreadPool;

    class TcpServer : boost::noncopyable
    {
        public:
            typedef boost::function<void(EventLoop *)> ThreadInitCallback;
            enum Option
            {
                kNoReusePort;
                kReusePort,
            };
            
            TcpServer(EventLoop *loop,
                      const InetAddress listenAddr,
                      const string &nameArg,
                      Option option = kNoReusePort);

            ~TcpServer();

            const string &hostport()const{ return hostport_;}
            const string &name()const {return name_;}
            EventLoop *getLoop() { return loop_;}

            //线程池的线程个数
            void setThreadNum(int numThreads);

            //设置线程池初始化回调函数
            void setThreadInitCallback(const ThreadInitCallback &cb)
            {
                threadInitCallback_ = cb;
            }

            //此线程池函数调用start()后有效
            boost::shared_ptr<EventLoopThreadPool> threadPool()
            {
                return threadPool_;
            }

            //启动server端开始监听
            void start();

            //设置连接回调
            void setConnectionCallback(const ConnectionCallback &cb)
            {
                connectionCallback_ = cb;
            }

            //设置消息回调
            void setMessageCallback(const MessageCallback &cb)
            {
                messageCallback_ = cb;
            }

            //设置写完成回调
            void setWriteCompleteCallback(const WriteCompleteCallback &cb)
            {
                writeCompleteCallback_ = cb;
            }
        private:
            //新连接的回调
            void newConnection(int sockfd,const InetAddress &peerAddr);
            //移除连接后的回调
            void removeConnection(const TcpConnectionPtr &conn);
            //在主循环中移除连接
            void removeConnectionInLoop(const TcpConnectionPtr &conn);

            typedef std::map<string,TcpConnectionPtr> connectionMap;

            EventLoop *loop_;            //所属的事件循环
            const string hostport_;      //本机端口
            const string name_;
            boost::scoped_ptr<Acceptor> acceptor; //连接管理对象
            boost::shared_ptr<EventLoopThreadPool> threadPool_;
            ConnectionCallback connectionCallback_;  //连接以后的回调
            MessageCallback messageCallback_;        //消息回调
            WriteCompleteCallback writeCompleteCallback_; //写完成回调
            ThreadInitCallback threadInitCallback_;  //线程初始化回调
            AtomicInt32 started_;

            int nextConnId_;
            ConnectionMap connections_; //保存所有的连接对象
    };
}
}


#endif
