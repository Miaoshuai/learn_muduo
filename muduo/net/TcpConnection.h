/*======================================================
    > File Name: TcpConnection.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月15日 星期日 17时19分47秒
 =======================================================*/

#ifndef MUDUO_NET_TCPCONNECTION_H
#define MUDUO_NET_TCPCONNEXTION_H

#include <muduo/base/StringPiece.h>
#include <muduo/base/Types.h>
#include <muduo/net/Callbacks.h>
#include <muduo/net/Buffer.h>
#include <muduo/net/InetAddress.h>

#include <boost/any.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

struct tcp_info;

namespace muduo
{
namespace net
{
    class Channel;
    class EventLoop;
    class Socket;

    class TcpConnection : boost::noncopyable,public boost::enable_shared_from_this<TcpConnection>
    {
        public:
          TcpConnection(EventLoop *loop,
                        const string &name,
                        int sockfd,
                        const InetAddress &localAddr,
                        const InetAddress &peerAddr);
          ~TcpConnection();

          //获取所属的循环
          EventLoop *getLoop()const{return loop_;}
          const string &name()const{return name_;}
          const InetAddress &localAddress()const{return localAddr_}
          const InetAddress &peerAddress()const{return peerAddr_;}
          bool connected()const{return state_ == kConnected;}
          bool disconnected()const{return state_ == kDisconnected;}

          bool getTcpInfo(struct tcp_info *)const;
          string getTcpInfoString()const;

          void send(const void *message,int len);
          void send(const StringPiece& message);

          void send(Buffer *message);
          void shutdown();

          void forceClose();
          void forceCloseWithDelay(double seconds);
          void setTcpNoDelay(bool on);

          void setContext(const boost::any & context)
          {context_ = context;}

          const boost::any &getContext()const
          { return context_;}

          boost::any *getContext()const
          { return &context_;}

          boost::any &getMutableContext()
          {
            return &context_;
          }
          //设置连接回调
          void setConnectionCallback(const ConnectionCallback &cb)
          {
            connectionCallback_ = cb;
          }

          void setMessageCallback(const MessageCallback &cb)
          {
            messageCallback_ = cb;
          }

          void setWriteCompleteCallback(const WriteCompleteCallback &cb)
          {
            writeCompleteCallback_ = cb;
          }

          void setHighWaterMarkCallback(const HighWaterMarkCallback &cb,size_t highWaterMark)
          {
            highWaterMarkCallback_ = cb;
            highWaterMark_ = highWaterMark;
          }

          Buffer *inputBuffer()
          {
            return &inputBuffer_;
          }

          Buffer *outputBuffer()
          {
            return &outputBuffer_;
          }

          void setCloseCallback(const CloseCallback &cb)
          {
            closeCallback_ = cb;
          }

          void connectEstablished();

          void connectDestroyed();

        private:
          enum StateE {kDisconnected,kConnecting,kConnected,kDisconnecting};
          void handleRead(Timestamp receiveTime);
          void handleWrite();
          void handleClose();
          void handleError();

          void sendInLoop(const StringPiece &message);
          void sendInLoop(const void *message,size_t len);
          void shutdownInLoop();

          void forceCloseInLoop();
          void setState(StateE s)
          {
            state_ = s;
          }
          const char *stateToString()const;

          EventLoop *loop_;         //所属的loop
          const string name_;       //map的key，TcpServer管理
          StateE state_;            //状态

          boost::scoped_ptr<Socket> socket_;
          boost::scoped_ptr<Channel> channel_;  //管理socket_
          const InetAddress localAddr_;
          const InetAddress peerAddr_;
          ConnectionCallback connectionCallback_;   //连接回调
          MessageCallback messageCallback_;         
          WriteCompleteCallback writeCompleteCallback_;
          HighWaterMarkCallback highWaterMarkCallback_;
          CloseCallback closeCallback_;     //关闭回调
          size_t highWaterMark_;            //高水位标志
          Buffer inputBuffer_;              //输入缓冲区
          Buffer outputBuffer_;             //输出缓冲区
          boost::any context_;

              
    };

    typedef boost::shared_ptr<TcpCOnnection> TcpConnectionPtr;
}
}
