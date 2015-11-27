/*======================================================
    > File Name: HttpContext.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月27日 星期五 15时09分18秒
 =======================================================*/

#ifndef MUDUO_NET_HTTP_HTTPCONTEXT_H
#define MUDUO_NET_HTTP_HTTPCONTEXT_H

#include <muduo/base/copyable.h>

#include <muduo/net/http/HttpRequest.h>

namespace muduo
{
namespace
{
    class Buffer;
    class HttpContext : public muduo::copyable
    {
        public:
            enum HttpRequestParseState
            {
                kExpectRequestLine,
                kExpectHeaders,
                kExpectBody,
                kGotAll,
            };

            HttpContext()
                :state_(kExpectRequestLine)
            {
            
            }
            
            bool parseRequest(Buffer *buf,Timestamp receiveTime);

            bool gotAll()const
            {
                return state_ == kGotAll;
            }

            void reset()
            {
                state_ = kExpectRequestLine;
                HttpRequest dummy;
                request_.swap(dummy);
            }

            const HttpRequest &request()const
            {
                return request_;
            }

            HttpRequest &request()
            {
                return request_;
            }
        private:
            //处理请求行
            bool processRequestLine(const char *begin,const char *end);

            HttpRequestParseState state_;
            HttpRequest request_;
            
    };
}
}
