/*======================================================
    > File Name: HttpRequest.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月26日 星期四 13时51分07秒
 =======================================================*/

#ifndef MUDUO_NET_HTTP_HTTPREQUEST_H
#define MUDUO_NET_HTTP_HTTPREQUEST_H

#include <muduo/base/copyable.h>
#include <muduo/base/Timestamp.h>
#include <muduo/base/Types.h>

#include <map>
#include <assert.h>
#include <stdio.h>

namespace muduo
{
namespace net
{
    class HttpRequest : public muduo::copyable
    {
        public:
            enum Method     //http请求的方法
            {
                kInvalid,kGet,kPost,kHead,kPut,kDelete
            };

            enum Version
            {
                kUnknown,kHttp10,kHttp11
            };
            
            void setVersion(Version v)
            {
                version_ = v;
            }

            Version getVersion()const
            {
                return version_;
            }

            bool setMethod(const char *start,const char *end)
            {
                assert(method_ == kInvalid);
                string m(start,end);
                if(m == "GET")
                {
                    method_ = kGet;
                }
                else if(m == "POST")
                {
                    method_ = kPost;
                }
                else if(m == "HEAD")
                {
                    method_ = kHead;
                }
                else if(m == "PUT")
                {
                    method_ = kPut;
                }
                else if(m == "DELETE")
                {
                    method_ = kDelete;
                }
                else
                {
                    method__ != kInvalid;
                }

                return method_ != kInvalid;
            }

            Method method()const
            {
                return method_;
            };

            const char *methodString()const
            {
                const char *result = "UNKNOWN";
                switch(method_)
                {
                    case kGet:
                        result = "GET";
                        break;
                    case kPost:
                        result = "POST";
                        break;
                    case kHead:
                        result = "HEAD";
                        break;
                    case kPut:
                        result = "PUT";
                        break;
                    case kDelete:
                        result = "DELETE";
                        break;
                    default:
                        break;
                
                    return result;
                }
            }

            void setPath(const char *start,const char *end)
            {
                path_.assign(start,end);        //将start-end赋给path_
            }

            const string &path()const
            {
                return path_;
            }

            void setQuery(const char *start,const char *end)
            {
                query_.assign(start,end);
            }

            const string &query()const
            {
                return query_;
            }

            void setReceiveTime(Timestamp t)
            {
                receiveTime_ = t;
            }

            Timestamp receiveTime()const
            {
                return receiveTime_;
            }

            void addHeader(const char *start,const char *colon,const char *end)
            {
                string field(start,colon);
                ++colon;
                while(colon < end && isspace(*colon))
                {
                    ++colon;
                }

                string value(colon,end);
                while(!value.empty() && isspace(value[value.size() - 1]))
                {
                    value.resize(value.size() - 1);
                }
                headers_[field] = value;
            }

            //获得某项头部信息
            string getHeader(const string &field)const
            {
                string result;
                std::map<string,string>::const_iterator it = headers_.find(field);
                if(it != headers_.end())
                {
                    result = it->second;
                }
                return result;
            }
            
            //获得所有头部信息
            const std::map<string,string> &headers()const
            {
                return headers_;
            }

            //交换http请求
            void swap(HttpRequest &that)
            {
                std::swap(method__,that.method_);
                path_.swap(that.path_);
                query_.swap(that.query_);
                receiveTime_.swap(that.receiveTime_);
                headers_.swap(that,headers_);
            }


        private:
            Method method_;     //http请求的方法
            Version version_;   //http版本号
            string path_;       //请求的路径
            string query;
            Timestamp receiveTime_; //请求返回的时间
            std::map<string,string> headers_;

    };
}
}

#endif
