/*======================================================
    > File Name: HttpServer.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月28日 星期六 11时30分25秒
 =======================================================*/

#include <muduo/net/http/HttpServer.h>

#include <mduuo/base/Logging.h>
#include <muduo/net/http/HttpContext.h>
#include <muduo/net/http/HttpRequest.h>
#include <muduo/net/http/HttpResponse.h>

#include <boost/bind.hpp>

using namespace muduo;
using namespace muduo::net;

namespace muduo
{
namespace net
{
namespace detail
{
    void defaultHttpCallback(const HttpRequest &,HttpResponse *resp)
    {
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setCLoseConnection(true);
    }


}    
}
}


HttpServer::HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &name,
                       TcpServer::Option option)
{
    server_.setConnectionCallback(
            boost::bind(&HttpServer::onConnection,this,_1));    //设置连接回调
    server_.setMessageCallback(
            boost::bind(&HttpServer::onMessage,this,_1,_2,_3)); //设置消息回调
}

HttpServer::~HttpServer()
{

}

void HttpServer::start()
{
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn)
{
    if(conn->connected())
    {
        conn->setContext(HttpContext());//设置连接的内容
    }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buf,
                           Timestamp receiveTime)
{
    HttpContext *context = boost::any_cast<HttpContext>(conn->getMutableContext()); //boost::any可以为任意类型
    if(!context->parseRequest(buf,receiveTime))
    {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if(context->gotAll())
    {
        onRequest(conn,context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const TcpConnectionPtr &conn,const HttpRequest &req)
{
    const string &connection = req.getHeader("Connection");
    bool close = connection == "close" ||
        (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    httpCallback_(req,&response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if(response.closeConnection())
    {
        conn->shutdown()
    }
}
































