/*======================================================
    > File Name: acceptor.cpp
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月21日 星期六 21时30分28秒
 =======================================================*/

#include <muduo/net/EventLoop.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <muduo/net/Channel.h>
#include <muduo/base/Timestamp.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/Acceptor.h>

int sock;
muduo::net::EventLoop *g_loop;


//设置handle中的newConnectionCallback
void cb1(int fd,const muduo::net::InetAddress &peerAddr)
{
    printf("hello\n");
}

int main(int argc,char **argv)
{
  muduo::net::EventLoop loop;
  g_loop = &loop;
  uint16_t port = 6666;
  muduo::net::InetAddress listenAddr("127.0.0.1",port);
  muduo::net::Acceptor acceptor(g_loop,listenAddr,true);
  acceptor.listen(); 
  acceptor.setNewConnectionCallback(cb1);       //设置新回调
  loop.loop();                                  //开始循环
  return 0;
}
