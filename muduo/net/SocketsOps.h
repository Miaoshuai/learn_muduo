/*======================================================
    > File Name: SocketsOps.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月02日 星期一 22时17分29秒
 =======================================================*/

#ifndef MUDUO_NET_SOCKETSOPS_H
#define MUDUO_NET_SOCKETSOPS_H

#include <arpa/inet.h>

namespace muduo
{
namespace net
{
namespace sockets
{
    //创建费阻塞文件描述符
    int createNonblockingOrDie();

    //建立连接
    int connect(int sockfd,const struct sockaddr_in &addr);

    //绑定套接字描述符
    void bindOrDie(int sockfd,const struct sockaddr_in &addr);

    //设置监听套接字
    void listenOrDie(int sockfd);

    //接受客户端
    int accept(int sockfd,struct sockaddr_in *addr);

    //读取数据
    ssize_t read(int sockfd,void *buf,ssize_t count);

    //读取块数据
    ssize_t readv(int sockfd,const struct iovec *iov,int iovcnt);

    //写入数据
    ssize_t write(int sockfd,const void *buf,size_t count);

    //关闭套接字描述符
    void close(int sockfd);

    //关闭写
    void shutdownWrite(int sockfd);

    void toIpPort(char *buf,size_t size,const struct sockaddr_in &addr);

    void toIp(char *buf,size_t size,const struct sockaddr_in &addr);

    void fromIpPort(const char *ip,uint16_t port,struct sockaddr_in *addr);

    int getSocketError(int sockfd);

    //类型转换
    const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr);
    struct sockaddr *sockaddr_cast(struct sockaddr_in *addr);
    const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr *addr);
    struct sockaddr_in *sockaddr_in_cast(struct sockaddr *addr);

    //获得本地地址
    struct sockaddr_in getLocalAddr(int sockfd);
    
    struct sockaddr_in getPeerAddr(int sockfd);

    bool isSelfConnect(int sockfd);
}
}
}

#endif
