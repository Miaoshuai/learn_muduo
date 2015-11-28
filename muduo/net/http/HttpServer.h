/*======================================================
    > File Name: HttpServer.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月28日 星期六 11时12分56秒
 =======================================================*/

#ifndef MUDUO_NET_HTTP_HTTPSERVER_H
#define MUDUO_NET_HTTP_HTTPSERVER_H

#include <muduo/net/TcpServer.h>
#include <boost/noncopyable.hpp>

namespace muduo
{
namespace net
{
    class HttpRequest;
    class HttpResponse;

    class HttpServer : boost::noncopyable
    {
        public:
            typedef boost::function<void (const HttpRequest &,HttpRequest *)> HttpCallback;

            HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &name,
                       TcpServer::Option option = TcpServer::kNoReusePort);
            ~HttpServer();
            
            EventLoop *getLoop()const
            {
                return server_.getLoop();
            }

            void setHttpCallback(const HttpCallback &cb)
            {
                httpCallback_ = cb;
            }

            void setThreadNum(int numThreads)
            {
                server_.setThreadNum(numThreads);
            }

            void start();
        private:
            void onConnection(const TcpConnectionPtr &conn);
            void onMessage(const TcpConnectionPtr &conn,
                           Buffer *buf,Timestamp receiveTime);

            void onRequest(const TcpConnectionPtr &,const HttpRequest &);

            TcpServer server_;
            HttpCallback httpCallback_;
    };
}
}
