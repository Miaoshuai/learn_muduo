/*======================================================
    > File Name: loop.cpp
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月21日 星期六 15时23分08秒
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
    printf("hello");
}

//放入queueInLoop
void cb2(void)
{
    printf(" world\n");
}

/*int listenFd(void)
{
    const char *ip = "127.0.0.1";
    int port = atoi("4444");

    struct sockaddr_in address;
    bzero(&address,sizeof(address));

    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);

    sock = socket(PF_INET,SOCK_STREAM,0);

    int ret = bind(sock,(sockaddr *)&address,sizeof(address));
    
    ret = listen(sock,9);

    return sock;
}*/

/*void readCallback()
{
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    printf("haha\n");
    g_loop->runInLoop(cb1);
    g_loop->queueInLoop(cb2);
    int connfd = accept(sock,(struct sockaddr *)&client,&len);
}*/



int main(int argc,char **argv)
{
  muduo::net::EventLoop loop;
  g_loop = &loop;
 //muduo::net::Channel channel(&loop,listenFd());
  
  //channel.setReadCallback(readCallback);        //设置读回调
  //channel.enableReading();                      //设置可读事件
  //loop.updateChannel(&channel);                 //加入epoll事件表
  muduo::net::InetAddress listenAddr(3333,false,false);
  muduo::net::Acceptor acceptor(g_loop,listenAddr,true);
  acceptor.listen(); 
  acceptor.setNewConnectionCallback(cb1);
  loop.loop();                                  //开始循环
  return 0;
}
