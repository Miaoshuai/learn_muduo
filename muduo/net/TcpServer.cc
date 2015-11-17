/*======================================================
    > File Name: TcpServer.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月16日 星期一 23时00分30秒
 =======================================================*/

#include <muduo/net/TcpServer.h>

#include <muduo/base/Logging.h>
#include <muduo/net/Acceptor.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/EventLoopThreadPool.h>
#include <muduo/net/SocketsOps.h>

#include <boost/bind.hpp>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

TcpServer::TcpServer(EventLoop *loop,
                     const InetAddress &listenAddr,
                     const string &nameArg,
                     Option option)
    :loop_(CHECK_NOTNULL(loop)),
    hostport_(listenAddr.toIpPort()),
    name_(nameArg),
    acceptor_(new Acceptor(loop,listenAddr,option == kReusePort)),
    threadPool_(new EventLoopThreadPool(loop,name_)),
    connectionCallback_(defaultConnectionCallback),
    messageCallback_(defaultMessageCallback),
    nextConnId_(1)
{
    acceptor_->setNewConnectionCallback(
            boost::bind(&TcpServer::newConnection,this,_1,_2));
}

TcpServer::~TcpServer()
{
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer[" << name_ <<"] destructing";

    for(ConnectionMap::iterator it(connections_begin());
            it != connections_end(); ++it)
    {
        TcpConnectionPtr conn = it->second;
        it->second.reset();
        conn->getLoop()->runInLoop(
                boost::bind(&TcpConnection::connectDestroyed,conn));
        conn.reset();
    }
}

//设置线程池的数量
void TcpServer::setThreadNum(int numThreads)
{
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}


//启动TcpServer
void TcpServer::start()
{
    if(started_.getAndSet(1) == 0)
    {
        threadPool_->start(threadInitCallback_);
        assert(!acceptor_->listenning());
        loop_->runInLoop(
                boost::bind(&Acceptor::listen,get_pointer(acceptor_)));
    }
}


void TcpServer::newConnection(int sockfd,const InetAddress &peerAddr)
{
    loop_->assertInLoopThread();
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[32];
    snprintf(buf,sizeof buf, ":%s#%d",hostport_c_str(),nextConnId_);
    ++nextConnId_;
    string connName = name_ buf;

    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    //创建连接对象
    TcpConnectionPtr conn(new TcpConnection(ioLoop,
                                           connName,
                                           sockfd,
                                           localAddr,
                                           peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
            boost::bind(&TcpServer::removeConnection,this,_1));
    ioLoop->runInLoop(boost::bind(&TcpServer::removeConnectionInLoop,this,conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn)
{
    loop_->runInLoop(boost::bind(&TcpServer::removeConnectionInLoop,this,conn))
}


void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn)
{
    loop_->assertInLoopThread();
    size_t n = connections_.erase(conn->name());
    void(n);
    assert( n == 1);
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
            boost::bind(&TcpConnection::connectionDestroyed,conn));
}





































