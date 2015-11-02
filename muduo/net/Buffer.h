/*======================================================
    > File Name: Buffer.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年11月02日 星期一 19时04分56秒
 =======================================================*/

#ifndef MUDUO_NET_BUFFER_H
#define MUDUO_NET_BUFFER_H

#include <muduo/base/copyable.h>
#include <muduo/base/StringPiece.h>
#include <muduo/base/Types.h>

#include <muduo/net/Endian.h>

#include <algorithm>
#include <vector>

#include <assert.h>
#include <string.h>

namespace muduo
{
namespace net
{
    class Buffer : public muduo::copyable
    {
        public:
            static const size_t kCheapPrepend = 8;
            static const size_t kInitialSize  = 1024;
            
            explicit Buffer(size_t initialSize = kInitialSize)
                :buffer_(initialSize + kCheapPrepend),
                readerIndex(kCheapPrepend),
                writerIndex_(kCheapPrepend)
            {
                assert(readableBytes() == 0);
                assert(writableBytes() == initialSize);
                assert(prependableBytes() == kCheapPrepend);
            }

            //交换俩块buffer
            void swap(Buffer &rhs)
            {
                buffer_.swap(rhs.buffer_);
                std::swap(readerIndex_,rhs.readerIndex_);
                std::swap(writerIndex_,rhs.writerIndex_);
            }

            //buffer中可读的字节数
            size_t readableBytes()const
            {
                return writerIndex_ - readerIndex_;
            } 

            //buffer中可写的字节
            size_t writableBytes()const
            {
                return buffer_.size - writerIndex_;
            }

            //预留空间
            size_t prependableBytes()const
            {
                return readerIndex_;
            }

            //读位置指针
            const char *peek()const
            {
                return begin() + readerIndex_;   
            }

            //找/r/n(每一次数据的终止标志)
            const char *findCRLF()const
            {
                const char *crlf = std::search(peek(),beginWrite(),KCRLF,KCRLF + 2);
                return crlf == beginWrite() ? NULL : crlf; //等于写的位置则说明到了缓冲区尾部，返回null
            }

            //从start到可写处
            const char *findCRLF(const char *start)const
            {
                assert(peek <= start);
                assert(start <= beginWrite());
                const char *crlf = std::search(start,beginWrite(),KCRLF,KCRLF+2);
                return crlf == beginWrite() ? NULL : crlf;   
            }

            //找\n
            const char *findEOL()const
            {
                //从peek指定区域的前可读个字节找\n
                const void *eol = memchr(peek(),'\n',readableBytes);
                return static_cast<const char *>(eol);
            }

            //从start处开始找\n
            const char *findEOL(const char *start)const
            {
                assert(peek() <= start);
                assert(start <= beginWrite);
                const void *eol = memchr(start,'\n',beginWrite() - start);
                return static<const char *>(eol);
            }
            
            //读操作之后移动读下标
            void retrieve(size_t len)
            {
                assert(len <= readableBytes);
                if(len < readableBytes)
                {
                    readerIndex_ += len;
                }

                else
                {
                    retrieveAll();
                }

            }

            void retrieveUntil(const char *end)
            {
                assert(peek() <= end);
                assert(end <= beginWrite());
                retrieve(end - peek());
            }

            void retrieveInt64()
            {
                retrieve(sizeof(int64_t));
            }

            void retrieveInt32()
            {
                retrieve(sizeof(int32_t));
            }

            void retrieveInt16()
            {
                retrieve(sizeof(int16_t));
            }

            void retrieveInt8(）
            {
                retrieve(sizeof(int8_t));        
            }

            void retrieveAll()
            {
                readerIndex_ = kCheapPrepend;
                writerIndex_ = kCheapPrepend;
            }

            //接受一定长度的字符串
            string retrieveAllAsString()
            {
                return retrieveAsString(readableBytes);
            }

            string retrieveAsString(size_t len)
            {
                assert(len <= readableBytes);
                //定义个result字符串接受
                string result(peek(),len);
                retrieve(len);
                return result;
            }

            StringPiece toStringPiece()const
            {
                return StringPiece(peek(),static_cast<int>(readableBytes));
            }

            //往缓冲区中添加字符串
            void append(const StringPiece &str)
            {
                append(str.data(),str.size());
            }

            void append(const char *data,size_t len)
            {
                ensureWritableBytes(len);
                std::copy(data,data + len,beginWrite());
                hasWritten(len);
            }

            void append(const void *data,size_t len)
            {
                append(static_cast<const char *>(data),len);
            }

            //确保写安全
            void ensureWritableBytes(size_t)
            {
                if(writableBytes() < len)
                {
                    makeSpace(len);
                }
                assert(writableBytes() >= len);
            }

            char *beginWrite()
            {
                return begin() + writerIndex_;
            }

            const char *beginWrite()const
            {
                return begin() + writerIndex_;
            }

            //跟新写下标
            void hasWritten()
            {
                assert(len <= writableBytes);
                writerIndex_ += len;
            }

            //擦除刚写的
            void unwrite(size_t len)
            {
                assert(len <= readableBytes);
                writerIndex_ -= len;
            }

            
            /// Append int64_t using network endian
            
            void appendInt64(int64_t x)
            {
                int64_t be64 = sockets::hostToNetwork64(x);
                append(&be64, sizeof be64);
            }

            
             /// Append int32_t using network endian
            
            void appendInt32(int32_t x)
             {
                int32_t be32 = sockets::hostToNetwork32(x);
                append(&be32, sizeof be32);
             }

             void appendInt16(int16_t x)
            {
                int16_t be16 = sockets::hostToNetwork16(x);
                append(&be16, sizeof be16);
            }

            void appendInt8(int8_t x)
            {
                append(&x, sizeof x);
            }

            
            /// Read int64_t from network endian
            
            /// Require: buf->readableBytes() >= sizeof(int32_t)
            int64_t readInt64()
            {
                 int64_t result = peekInt64();
                 retrieveInt64();
                return result;
            }

            
            /// Read int32_t from network endian
            
            /// Require: buf->readableBytes() >= sizeof(int32_t)
            int32_t readInt32()
            {
                 int32_t result = peekInt32();
                 retrieveInt32();
                 return result;
            }

             int16_t readInt16()
            {
                int16_t result = peekInt16();
                 retrieveInt16();
                return result;
            }

            int8_t readInt8()
            {
                int8_t result = peekInt8();
                retrieveInt8();
                return result;
            }

            
            /// Peek int64_t from network endian
            
            /// Require: buf->readableBytes() >= sizeof(int64_t)
             int64_t peekInt64() const
            {
                assert(readableBytes() >= sizeof(int64_t));
                 int64_t be64 = 0;
                ::memcpy(&be64, peek(), sizeof be64);
                return sockets::networkToHost64(be64);
            }

            ///
            /// Peek int32_t from network endian
            ///
            /// Require: buf->readableBytes() >= sizeof(int32_t)
             int32_t peekInt32() const
            {
                 assert(readableBytes() >= sizeof(int32_t));
                 int32_t be32 = 0;
                ::memcpy(&be32, peek(), sizeof be32);
                return sockets::networkToHost32(be32);
            }

             int16_t peekInt16() const
             {
                assert(readableBytes() >= sizeof(int16_t));
                int16_t be16 = 0;
                 ::memcpy(&be16, peek(), sizeof be16);
                return sockets::networkToHost16(be16);
            }

            int8_t peekInt8() const
            {
                assert(readableBytes() >= sizeof(int8_t));
                int8_t x = *peek();
                return x;
            }

            ///
            /// Prepend int64_t using network endian
            ///
            void prependInt64(int64_t x)
            {
                int64_t be64 = sockets::hostToNetwork64(x);
                prepend(&be64, sizeof be64);
            }

            ///
             /// Prepend int32_t using network endian
            ///
            void prependInt32(int32_t x)
            {
                 int32_t be32 = sockets::hostToNetwork32(x);
                prepend(&be32, sizeof be32);
            }

             void prependInt16(int16_t x)
            {
                 int16_t be16 = sockets::hostToNetwork16(x);
                 prepend(&be16, sizeof be16);
            }

            void prependInt8(int8_t x)
            {
                prepend(&x, sizeof x);
            }

            void prepend(const void* /*restrict*/ data, size_t len)
            {
                assert(len <= prependableBytes());
                readerIndex_ -= len;
                const char* d = static_cast<const char*>(data);
                std::copy(d, d+len, begin()+readerIndex_);
            }

            void shrink(size_t reserve)
            {
                // FIXME: use vector::shrink_to_fit() in C++ 11 if possible.
                 Buffer other;
                other.ensureWritableBytes(readableBytes()+reserve);
                other.append(toStringPiece());
                swap(other);
            }

            size_t internalCapacity() const
            {
                return buffer_.capacity();
            }

            /// Read data directly into buffer.
            ///
            /// It may implement with readv(2)
            /// @return result of read(2), @c errno is saved
            ssize_t readFd(int fd, int* savedErrno);





        private:
            char *begin()   //获取vector起始点的地址,然后和读写下标的整数值相加就是读写位置的地址了，而如果直接把读写位置写成地址的话，很容易失效，这也是为什么我们用整形值来记录读写地址的原因
            {
                return &*buffer_.begin();
            }

            const char *begin()const
            {
                return &*buffer_.begin();
            }

            void makeSpace(size_t len)  //增大vector
            {
                if(writableBytes() + prependableBytes() < len + kCheapPrepend) //可写空间和移动中腾出的空间是否小于写入长度和预留空间的长度和   
                {
                    //说明缓存空间真是不够不够用了
                    buffer_.resize(writerIndex_ + len);
                }
                else
                {
                    assert(kCheapPrepend < readerIndex);
                    size_t readable = readableBytes();
                    //将现有数据腾挪到kCheapPrepend位置
                    std::copy(begin() + readerIndex_,
                              begin() + writerIndex_,
                              begin() + kCheapPrepend);
                    readerIndex_ = kCheapPrepend;
                    writerIndex_ = readerIndex_ + readable;
                    assert(readable == readableBytes());
                       
                }
            }
        private:
            std::vector<char> buffer_;        //容器来充当缓冲
            size_t readerIndex_;              //读位置下标
            size_t writerIndex_;              //写位置下标

            static const char kCRLF[];        //一段内容结束标志  
    };
}
}
