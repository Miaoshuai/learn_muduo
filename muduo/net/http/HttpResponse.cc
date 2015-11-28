/*======================================================
    > File Name: HttpResponse.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月28日 星期六 10时30分40秒
 =======================================================*/

#include <muduo/net/http/HttpResponse.h>
#include <muduo/net/Buffer.h>

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

void HttpResponse::appendToBuffer(Buffer *output)const
{
    char buf[32];
    snprintf(buf,sizeof buf,"HTTP/1.1 %d",statusCode_);
    output->append(buf);
    output->append(statusMessage_);
    output->append("\r\n");

    if(closeConnection_)
    {
        output->append("Connection:close\r\n");
    }
    else
    {
        snprintf(buf,sizeof buf,"Content-Length:%zd\r\n",body_.size());
        output->append(buf);
        output->append("Connection: Keep-Alive\r\n");
    }

    for(std::map<string,string>::const_iterator it = headers_begin();
        it != headers_.end();
        ++it)
    {
        output->append(it->first);
        output->append(": ");
        output->append(it->second);
        output->append("\r\n");
    }

    output->append(""\r\n);
    output->append(body);
}
