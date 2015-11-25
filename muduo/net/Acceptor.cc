/*======================================================
    > File Name: Acceptor.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月15日 星期日 14时18分12秒
 =======================================================*/

#include <muduo/net/Acceptor.h>

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/SocketsOps.h>

#include <boost/bind.hpp>

#include <errno.h>
#include <fcntl.h>

using namespace muduo;
using namespace muduo::net;

Acceptor::Acceptor(EventLoop *loop,const InetAddress & listenAddr,bool reuseport)
    :loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDie()),
    acceptChannel_(loop,acceptSocket_.fd()),
    listenning_(false),
    idleFd_(::open("/dev/null",O_RDONLY | O_CLOEXEC))
{
    assert(idleFd_ >= 0);
    //设置监听socket
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reuseport);  //端口重用
    acceptSocket_.bindAddress(listenAddr);  //绑定监听套接字
    acceptChannel_.setReadCallback(boost::bind(&Acceptor::handleRead,this));    //设置channel的读回调
}

Acceptor::~Acceptor()
{   
    acceptChannel_.disableAll();
    acceptChannel_.remove();
    ::close(idleFd_);    
}


void Acceptor::listen()
{
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen();                 //开始监听
    acceptChannel_.enableReading();         //设置可读事件
}

//读回调
void Acceptor::handleRead()
{
    loop_->assertInLoopThread();
    InetAddress peerAddr;

    //接受新连接，如果是短链接可以尝试用循环接受
    int connfd = acceptSocket_.accept(&peerAddr);

    if(connfd > 0)
    {
        if(newConnectionCallback_)
        {
            //新连接到来时调用newcallback
            newConnectionCallback_(connfd,peerAddr);
        }
        else
        {
            sockets::close(closefd);
        }
    }

    else
    {
        LOG_SYSERR << "in Acceptor::handleRead";

        if(errno == EMFILE) //文件描述符用完，新连接不能处理
        {
            ::close(idleFd_);   //关闭此空闲文件描述符
            idleFd_ = ::accept(acceptSocket_.fd(),NULL,NULL);   //获取连接
            :close(idleFd_);    //关闭新连接
            idleFd_ = ::open("/dev/null",O_RDONLY | O_CLOEXEC); //再次占用该文件描述符
        }
    }
    
        
}





























