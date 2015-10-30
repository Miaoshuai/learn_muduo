/*======================================================
    > File Name: AsyncLogging.cc
    > Author: MiaoShuai
    > E-mail:  
    > Other :  
    > Created Time: 2015年10月29日 星期四 20时34分19秒
 =======================================================*/

#include "AsyncLogging.h"
#include "LogFile.h"
#include "Timestamp.h"

#include <stdio.h>

using namespace muduo;

::AsyncLongging::AsyncLongging(const string &basename,
                                size_t roolSize,
                                int flushInterval)
    :flushInterval_(flushInterval),
    running_(false),
    basename_(basename),
    roolSize_(roolSize),
    thread_(boost::bind(&AsyncLongging::threadFunc,this),"Logging"),
    latch_(1),
    mutex_(),
    cond_(mutex_),
    currentBuffer_(new Buffer),
    nextBuffer_(new Buffer),
    buffers_()
{
    //初始化清空缓冲区
    currentBuffer_->bzero();
    nextBuffer_->bzero();

    //确定最大缓冲区块的个数
    buffers_.reserve(16);
}

void AsyncLongging::append(const char *logline,int len)
{
    //加锁
    muduo::MutexLockGuard lock(mutex_);
    
    //如果当前缓冲区块剩余长度大于len
    if(currentBuffer_->avail() > len)
    {
        //添加日志到缓冲区中
        currentBuffer_->append(logline,len);
    }

    else
    {
        //release函数返回一个指针，并将currentBuffer_置为空，这也是boost指针容器的特点:他对指针所指内容为独占，并前容器在析构时也会清理指针
        buffers_.push_back(currentBuffer_.release());
        
        //如果备用的额nextBuffer_可用则将其内存块转交给currentBuffer_
        if(nextBuffer_)
        {
            //此处使用move没有发生拷贝，而只是发生指针所指内存的控制权的交接
            currentBuffer_ = boost::ptr_container::move(nextBuffer_);
        }
        
        //否则说明当前没有空闲的缓冲块了，需要重新申请缓冲块
        else
        {
            //用新申请的缓冲块重置当前缓冲块
            currentBuffer_.reset(new Buffer);
        }
        
        //将参数传来的日志内容写入当前缓冲中
        currentBuffer_->append(logline,len);
        //由于有了新生成的写满的缓冲块，所以通知后端线程可以往磁盘写了
        cond_.notify();       
    }
}

void AsyncLongging::threadFunc()
{
    assert(running_ == true);
    
    //减少计数器的值
    latch_.countDown();
    
    //定义LogFile对象，用于用于输入文件的对象
    LogFile output(basename_,rollSize_,false);
    //申请俩快新的缓冲块
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);

    newBuffer1->bzero();
    newBuffer2->bzero();
    
    //保存可写缓冲块的容器
    BufferVector buffersToWrite;
    //最大存储16块缓冲
    buffersToWrite.reserve(16);

    while(running_)
    {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());
        {
            muduo::MutexLockGuard lock(mutex_);
            if(buffers_.empty())
            {
                //等待我们在AsyncLogging中传的刷新时间3s，当然也可能会被当前缓冲满了之后就唤醒
                cond_.waitForSeconds(flushInterval_);
            }

            //等待结束，将当前缓冲加入可写队列，准备写文件
            buffers_.push_back(currentBuffer_.release());

            //将备用缓存newBuffer1的控制权交给当前缓冲
            currentBuffer_ = boost::ptr_container:move(newBuffer1);

            //将二者交换，后面的代码我们就可以安全的访问bufferToWrite了
            if(!nextBuffer_)
            {
                //将nextBuffer_换位new2
                nextBuffer_ = boost::ptr_container::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        //如果容器的当前保存缓冲块的大小超过25，则有可能程序陷入死循环或其他问题，选择丢掉一些日志
        if(buffersToWrite.size() > 25)
        {
          char buf[256];
          snprintf(buf,sizeof buf,"Dropped log messages at %s,%zd larger buffers\n"
                  Timestamp::now().toFormattedString.c_str
                  buffersToWrite.size()-2);     //这里的size-2是我们想丢弃除前俩块外剩下的所有块
          fputs(buf,stderr);

          //把丢信息的报告写到日志中
          output.append(buf,static<int>(strlen(buf)));
          
          //将要丢弃的块从vector中删除
          buffersToWrite.earse(buffersToWrite.begin() + 2,buffersToWrite.end());
        }
        
        //将BufferVector中的缓冲块内容写到文件中
        for(size_t i = 0; i < buffersToWrite.size();++i)
        {
            output.append(buffersToWrite[i].data(),buffersToWrite[i].length());
        }

        //将容器中多于俩块的部分删除并销毁,因为我们整个日志有4块缓存就好，删除新分配的

        //如果newBuffer1被使用了，则将vector中已在上述循环写过了的缓冲块赋给它
        if(!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = buffersToWrite.pop_back();
            newBuffer1->reset();
        }


        if(!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        //清空vector并将刷在磁盘里
        buffersToWrite.clear();
        output.flush();
    } 
    output.flush();
}





































