/*======================================================
    > File Name: TcpConnection.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月16日 星期一 13时47分41秒
 =======================================================*/

#include <muduo/net/TcpConnection.h>

#include <muduo/base/Logging.h>
#include <muduo/base/WeakCallback.h>
#include <muduo/net/Channel.h>
#include <boost/bind.hpp>
#include <errno.h>

using namespace muduo;
using namespace muduo::net;

//默认连接回调
void muduo::net::defaultConnectionCallback(const TcpConnectionPtr &conn)
{
    LOG_TRACE << conn->localAddress().toIpPort() << "->"
              << conn->peerAddress().toIpPort() << " is"
              << (conn->connected() ? "UP" : "Down");   
}

//默认消息回调
void muduo::net::defaultMessageCallback(const TcpConnectionPtr &,
                                        Buffer *buf,
                                        Timestamp)
{
    buf->retrieveAll();
}

TcpConnectionPtr::TcpConnection(EventLoop *loop,
                                const string &nameArg,
                                int sockfd,const InetAddress &localAddr,
                                const InetAddress &peerAddr
                                )
    :loop_(CHECK_NOTNULL(loop)),
    name_(nameArg),
    state_(kConnecting),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop,sockfd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr),
    highWaterMark_(64*1024*1024)
{
    channel_->setReadCallback(boost::bind(&TcpConnection::handleRead,this,_1));
    channel_->setWriteCallback(boost::bind(&TcpConnection::handleWrite,this));
    channel_->setCloseCallback(boost::bind(&TcpConnection::handleClose,this));
    channel_->setErrorCallback(boost::bind(&TcpConnection::handleError,this));

    LOG_DEBUG << "TcpConnection::ctor[" << name_ <<"] at" <<this
              << " fd=" << sockfd;
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor["<< name_ <<"] at" <<this
              <<" fd = " << channel_->fd()
              << "state=" << stateToString();
    assert(state_ == kDisconnected);
}

//是否获得TCP连接的信息
bool TcpConnection::getTcpInfo(struct tcp_info *tcpi)const
{
    return socket_->getTcpInfo(tcpi);
}

string TcpConnection::getTcpInfoString()const
{
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf,size buf);
    return buf;
}

//无buffer发送数据
void TcpConnection::send(const void *data,int len)
{
    send(StringPiece(static_cast<const char *>(data)),len);
}

void TcpConnection::send(const StringPiece &message)
{
    if(state_ == kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(message);            //在主循环中发
        }
        else
        {
            loop_->runInLoop(boost::bind(&TcpConnection::sendInLoop,this,message.as_string()));
        }
    }
}


//有buffer发送数据
void TcpConnection::send(Buffer *buf)
{
    if(state_ == kConnected)
    {
        if(loop_->isInLoopThread())
        {
            sendInLoop(buf->peek(),buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            loop_->runInLoop(boost::bind(&TcpConnection::sendInLoop,this,buf->retrieveAllString()))
        }
    }
}


//在主循环中发送信息的函数
void TcpConnection::sendInLoop(const StringPiece &message)
{
    sendInLoop(message.data(),message.size());
}

void TcpConnection::sendInLoop(const void *data,size_t len)
{
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if(state_ == kDisconnected)
    {
        LoG_WARN << "disconnected,give up writeing";
        return ;
    }

    //如果输出队列没人在写，那么可以直接写
    if(!channel_->isWriting() && outputBuffer_.readableBytes() == 0)
    {
        nwrote = sockets::write(channel_->fd,data,len);
        if(nwrote >= 0)
        {
            remaining = len - nwrote;
            //如果刚才write写完了则调用写完成回调
            if(remaining == 0 && writeCompleteCallback_)
            {
                loop_->queueInLoop(boost::bind(writeCompleteCallback_,shared_from_this()));
            }
        }
        else
        {
            nwrote = 0;
            if (errno == EPIPE || errno == ECONNRESET)
            {
                faultError = true;
            }
        }
    }

    assert(remaining <= len);
    //有可能元数据字节没执行上面的直接往套接字写
    if(!faultError && remaining > 0)
    {
        size_t oldLen = outputBuffer_.readableBytes();  //buffer中已有的字节数

        //所要写的数据超过所设的高水位
        if(oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ && highWaterMarkCallback_)
        {
            loop_->queueInLoop(boost::bind(highWaterMarkCallback_,shared_from_this(),oldLen + remaining));   
        }
        outputBuffer_.append(static_cast<const char *>(data) + nwrote,remaining);   //将剩余的数据加入输出buffer中
        if(!channel_->isWriting())
        {
            channel_->enableWriting();  //由于套接字发送缓冲区已满，注册可写事件
        }
    }
}

//断开连接
void TcpConnection::shutdown()
{
    if(state_ == kConnected)
    {
        if(state_ == kConnected)
        {
            setState(kDisconnecting);
            loop_->runInLoop(boost::bind(&TcpConnection::shutdownInLoop,this));
        }
    }
}


void TcpConnection::shutdownInLoop()
{
    loop_->assertInLoopThread();
    if(!channel_->isWriting())
    {
        socket_->shutdownWrite();
    }
}


//服务器主动关闭套接字
void TcpConnection::forceClose()
{
    if(state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->queueInLoop(boost::bind(&TcpConnection::forceCloseInLoop,shared_from_this()));
    }
}


void TcpConnection::forceCloseWithDelay(double seconds)
{
    if(state_ == kConnected || state_ == kDisconnecting)
    {
        setState(kDisconnecting);
        loop_->runAfter(seconds,
                        makeWeakCallback(shared_from_this(),&TcpConnection::forceClose));
    }
}

void TcpConnection::forceCloseInLoop()
{
    loop_->assertInLoopThread();
    if(state_ == kConnected || state_ == kDisconnecting)
    {
        handleClose();
    }
}


//获取状态的字符串
const char *TcpConnection::stateToString()const
{
    switch(state_)
    {
        case kDisconnected:
            return "kDisconnected";
        case kConnecting:
            return "kConnecting";
        case kConnected:
            return "kConnected";
        case kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

//是否关闭nagle算法
void TcpConnection::setTcpNoDelay(bool on)
{
    socket_->setTcpNoDelay(on);
}

void TcpConnection::connectEstablished()
{
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}


//销毁连接
void TcpConnection::connectDestroyed()
{
    loop_->assertInLoopThread();
    if(state_ == kConnected)
    {
        setState(kDisconnecting);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
    }

    channel_->remove();
}


//处理读
void TcpConnection::handleRead(Timestamp receiveTime)
{
    loop_->assertInLoopThread();
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd,&savedErrno);  //先读到buffer中

    if(n > 0)
    {
        //通知应用，让其从buffer中读取数据
        messageCallback_(shared_from_this(),&inputBuffer_,receiveTime);
    }
    else if(n == 0)
    {
        handleClose();
    }
    else
    {
        errno  = savedErrno;
        LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
}


//直接往套接字写没写完，buffer里有数据，才调用此
void TcpConnection::handleWrite()
{
    loop_->assertInLoopThread();
    if(channel_->isWriting())
    {
        ssize_t n = sockets::write(channel_->fd(),
                                   outputBuffer_.peek(),
                                   outputBuffer_.readableBytes());
        if(n > 0)
        {
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0)
            {
                //关掉可写事件，否则可能产生busy loop
                channel_->disableWriting();
                //调用写完成回调
                if(writeCompleteCallback_)
                {
                    loop_->queueInLoop(boost::bind(writeCompleteCallback_,shared_from_this()));//在主I/O线程中完成
                }
                if(state_ == kDisconnecting)
                {
                    shutdownInLoop();
                }
            }
        }
        else
        {
            LOG_SYSERR << "TcpConnection::handleWrite";
        }
        
    }
    else
    {
        LOG_TRACE << "Connection fd = " << channel_->fd()
                  << " is down,no more writing";
    }
}

//处理关闭
void TcpConnection::handleClose()
{
    loop_->assertInLoopThread();
    LOG_TRACE << " fd = " << channel_->fd() << " state = " << stateToString();
    assert(state_ == kConnected || state_ == kDisconnecting);

    setState(kDisconnected);
    channel_->disableAll();     //清空事件

    TcpConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);

    closeCallback_(guardThis);
}

//处理错误
void TcpConnection::handleError()
{
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError [" << name_
              <<"] -SO_ERROR =" << err << "" strerror_tl(err);
}
















































































































































































































