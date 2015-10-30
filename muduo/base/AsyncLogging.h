/*======================================================
    > File Name: AsyncLogging.h
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年10月28日 星期三 16时01分12秒
 =======================================================*/

#ifndef MUDUO_BASE_ASYNCLONGGING_H
#define MUDUO_BASE_ASYNCLONGGING_H

#include "BlockingQueue.h"
#include "BoundedBlockingQueue.h"
#include "CountDownLatch.h"
#include "Thread.h"

#include "Logging.h"

#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace muduo
{
    class AsyncLongging : boost::noncopyable
    {
        public:
            AsyncLongging(const string &basename,size_t roolSize,int flushInterval = 3);

            ~AsyncLongging()
            {
                if(running_)
                {
                    stop();
                }
            }

            //添加日志
            void append(const char *logline,int len);
            
            //启动异步日志
            void start()
            {
                running_ = true;
                thread_.start();
                latch_.wait();
            }
            
            //停止异步日志
            void stop()
            {
                running_ = false;
                cond_.notify();
                thread_.join();
            }

        private:
            //禁止拷贝和复制操作
            AsyncLongging(const AsyncLongging &);
            void operator=(const AsyncLongging &);

            void threadFunc();

            typedef muduo::detail::FixedBuffer<muduo::detail::kLargeBuffer> Buffer;
            typedef boost::ptr_vector<Buffer> BufferVector;
            //auto为vector所包含元素的指针类型
            typedef BufferVector::auto_type BufferPtr;
        
            const int flushInterval_;
            bool running_;
            string basename_;                   //程序的名字argv[0]
            size_t roolSize_;                   //滚动，每写满多大文件就换下一个文件
            muduo::Thread thread_;              //自定义线程类
            muduo::CountDownLatch latch_;       //辅助同步的类，详情请看muduo源码里的CountDownLatch.h类
            muduo::MutexLock mutex_;            //封装的互斥所
            muduo::Condition cond_;             //封装的条件变量
            BufferPtr currentBuffer_;           //当前缓存块
            BufferPtr nextBuffer_;              //替补缓存块
            BufferVector buffers_;              //保存缓存快的集合
    }
}
