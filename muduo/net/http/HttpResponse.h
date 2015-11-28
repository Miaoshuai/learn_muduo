/*======================================================
    > File Name: HttpResponse.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月28日 星期六 09时15分06秒
 =======================================================*/

#ifndef MUDUO_NET_HTTPRESPONSE_H
#define MUDUO_NET_HTTPRESPONSE_H

#include <muduo/base/copyable.h>
#include <muduo/base/Types.h>

#include <map>

namespace muduo
{
namespace net
{
    class Buffer;
    class HttpResponse : public muduo::copyable
    {
        public:
            enum HttpStatusCode
            {
                kUnknow,
                k200Ok = 200,
                k301MovedPermanently = 301,
                k404NotFound = 404,
            };
            explicit HttpResponse(bool close)
                :statusCode_(kUnknow),
                closeConnection_(close)
            {
            
            }

            void setStatusCode(HttpStatusCode code)
            {
                statusCode_ = code;
            }

            void setStatusMessage(const string &message)
            {
                statusMessage_ = message;
            }

            void setCloseConnection(bool on)
            {
                closeConnection_ = on;
            }

            bool closeConnection()const
            {
                return closeConnection_;
            }

            void setContentType(const string &contentType)
            {
                addHeader("Content-Type",contentType);
            }

            void addHeader(const string &key,const string &value)
            {
                headers_[key] = value;
            }

            void setBody(const string &body)
            {
                body_ = body;
            }

            void appendToBuffer(Buffer *output)const;

        private:
            std::map<string,string> headers_;
            HttpStatusCode statusCode_;
            string statusMessage_;
            bool closeConnection_;
            string body_;
    }
}
}
