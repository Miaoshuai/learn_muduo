/*======================================================
    > File Name: HttpContext.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月27日 星期五 16时06分06秒
 =======================================================*/

#include <muduo/net/Buffer.h>
#include <muduo/net/http/HttpContext.h>

using namespace muduo;
using namespace muduo::net;

//解析请求行
bool HttpContext::processRequestLine(const char *begin,const char *end)
{
    bool succeed = false;
    const char *start = begin;
    const char *space = std::find(start,end,'');//找到空格的位置

    if(space != end && request_.setMethod(start,space))//将start到空格的值赋给请求方式
    {
        start = space + 1;
        space = std::find(start,end,'');//继续寻找下个空格
        if(space != end)
        {
            const char *question = std::find(start,space,'?');//看url里是否传了值
            if(question != space)
            {
                request_.setPath(start,question);//设置路径
                request_.setQuery(question,space);//设置传值的内容

            }
            else
            {
                request_.setPath(start,space);//请求的地址
            }
            start = space + 1;
            
            succeed = end-start == 8 && std::equal(start,end - 1,"HTTP/1.");//是否为http协议
            if(succeed)
            {
                if(*(end - 1) == '1')
                {
                    request_.setVersion(HttpRequest::kHttp11);//给版本类型赋值
                }
                else if(*(end -1) == '0')
                {
                    request_.setVersion(HttpRequest::kHttp10);
                }

                else
                {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}

bool HttpContext::parseRequest(Buffer *buf,Timestamp receiveTime)
{
    bool ok = true;
    bool hasMore = true;
    while(hasMore)
    {
        if(state_ == kExpectRequestLine)
        {
            const char *crlf = buf->findCRLF();
            if(crlf)
            {
                ok = processRequestLine(buf->peek(),crlf);//解析请求行
                if(ok)
                {
                    request_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf +2);
                    state_ = kExpectHeaders;

                }
                else
                {
                    hasMore = false;
                }
            }
            else
            {
                hasMore = false;
            }
        }
        else if(state_ == kExpectHeaders)//解析头部
        {
            const char *crlf = buf->findCRLF();
            if(crlf)
            {
                const char *colon = std::find(buf->peek(),crlf,':');
                if(colon != crlf)
                {
                    request_.addHeader(buf->peek(),colon,crlf);
                }
                else
                {
                    state_ = kGotAll;
                    hasMore = false;
                }
                buf->retrieveUntil(crlf + 2);
            }
            else
            {
                hasMore = false;
            }
        }
        else if(state_ == kExpectBody)//解析body
        {
        
        }
    }
    return ok;
}
