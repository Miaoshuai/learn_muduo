/*======================================================
    > File Name: Buffer.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月02日 星期一 23时14分09秒
 =======================================================*/

#include <muduo/net/Buffer.h>

#include <muduo/net/SocketsOps.h>

#include <errno.h>
#include<sys/uio.h> //readv

using namespace muduo;
using namespace muduo::net;

const char Buffer::KCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

ssize_t Buffer::readFd(int fd,int *savedErrno)
{
    //栈额外空间
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = writableBytes();

    vec[0].iov_base = begin() + ::writerIndex_;
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    //当vector够用时不用栈空间
    const int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = sockets::readv(fd,vec,iovcnt);
    if(n < 0)
    {
        *savedErrno = errno;
    }
    else if(implicit_cast<size_t>(n) <= writable)
    {
        writerIndex_ += n;
    }
    else
    {
        writerIndex_ = buffer_.size();
        append(extrabuf,n - writable);
    }

    return n;
}
