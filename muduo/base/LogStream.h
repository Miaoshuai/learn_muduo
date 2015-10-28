/*======================================================
    > File Name: LogStream.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年10月28日 星期三 21时14分31秒
 =======================================================*/

#ifndef MUDUO_BASE_LOGSTREAM_H
#define MUDUO_BASE_LOGSTREAM_H

#include "StringPiece.h"
#include "Types.h"
#include <assert.h>
#include <string.h> //为了memcpy
#ifndef MUDUO_STD_STRING
#include <string>
#endif

#include <boost/noncopyable.hpp>

namespace muduo
{
    namespace detail
    {
        const int kSmallBuffer = 4000;
        const int kLargeBuffer = 4000*1000;

        template<int SIZE>
        //固定大小的buffer
        class FixedBuffer : boost::noncopyable
        {
            public:
                FixedBuffer()
                    :cur_(data_)
                {
                    setCookie(cookieStart);
                }

                ~FixedBuffer()
                {
                    setCookie(cookieEnd);
                }
                //向缓冲区增加东西
                void append(const char *buf,size_t len)
                {
                    //比static_cast弱的转化
                    //剩余长度够得话就进行复制
                    if(implicit_cast<size_t>(avail() > len))
                    {
                        memcpy(cur_,buf,len);
                        cur_ += len;
                    }   
                }

                const char *data()const
                {
                    return data_;
                }

                //返回当前保存数据的长度
                int length()const
                {
                    return static_cast<int>(cur_ - data_);
                }

                //返回当前指针位置
                char *cur_ current()
                {
                    return cur_;
                }

                //返回剩余空间
                int avail()const
                {
                    return static_cast<int>(end() - cur_);
                }

                //增加当前指针位置
                void add(size_t len)
                {
                    cur_ += len;
                }

                //重置当前buffer位置指针
                void reset()
                {
                    cur_ = data_;
                }

                //清空当前缓冲
                void bzero()
                {
                    bzero(data_,sizeof data_);
                }

                //也可以通过GDB获得
                const char *debugString();

                //设置cookie
                void setCookie(void (*cookie)())
                {
                    cookie_ = cookie;
                }

                //可以用于单元测试
                string toString()const
                {
                    return string(data_,length());
                }

                StringPiece toStringPiece()const
                {
                    return StringPiece(data_,length());
                }
            private:
                const char* end()const
                {
                    return data_ + sizeof data_;
                }
                
                static void cookieStart();
                static void cookieEnd();

                void (*cookie_)();
                char data_[SIZE];
                char ×cur_;

        };
    }

    class LogStream : boost::noncopyable
    {
        typedef LogStream self;
        public:
            typedef detail::FixedBuffer<detail::kSmallBuffer> Buffer;

            self &operator<<(bool v)
            {
                buffer_.append(v ? "1" : "0",1);
                return *this;
            }

            self &operator<<(short);
            self &operator<<(unsigned short);
            self &operator<<(int);
            self &operator<<(unsigned int);
            self &operator<<(long);
            self &operator<<(unsigned long);
            self &operator<<(long long);
            self &operator<<(unsigned long long);

            self &operator<<(const void*);

            self &operator<<(float v)
            {
                *this << static_cast<double>(v);
                return *this;
            }

            self &operator<<(double);

            self &operator<<(char v)
            {
                buffer_.append(&v,1);
                return *this;
            }

            self &operator<<(const char *str)
            {
                if(str)
                {
                    buffer_.append(str,strlen(str));
                }
                else
                {
                    buffer_.append("(null)",6);
                }
                return *this;
            }

            self & operator<<(const unsigned char *str)
            {
                return operator<<(reinterpret_cast<const char *>(str));
            }

            self &operator<<(const string &v)
            {
                buffer_.append(v.c_str(),v.size());
                return *this;
            }

            self &operator<<(const StringPiece &v)
            {
                buffer_.append(v.data(),v.size());
                return *this;
            }

            self &operator<<(const Buffer &v)
            {
                *this << v.toStringPiece();
                return *this;
            }

            void append(const char *data,int len)
            {
                buffer_.append(data,len);
            }

            const Buffer &buffer()const
            {
                return buffer_;
            }

            void resetBuffer()
            {
                buffer_.reset();
            }
        private:
            void staticCheck();
            template<typename T>
            void formatInteger(T);

            Buffer buffer_;
            
            static const int kMaxNumericSize = 32;       
        };
    
        class Fmt
        {
            public:
                template<typename T>
                Fmt(const char *fmt,T val);

                const char *data()const
                {
                    teturn buf_;
                }

                int length()const
                {
                    return length_;
                }
            private:
                char buf_[32];
                int length_;
        };


        inline LogStream &operator<<(LogStream &s,Fmt &fmt)
        {
            s.append(fmt.data(),fmt.length());
            return s;
        }
}

#endif
