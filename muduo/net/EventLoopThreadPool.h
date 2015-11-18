// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_EVENTLOOPTHREADPOOL_H
#define MUDUO_NET_EVENTLOOPTHREADPOOL_H

#include <muduo/base/Types.h>

#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace muduo
{

namespace net
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : boost::noncopyable
{
 public:
  typedef boost::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg);
  ~EventLoopThreadPool();

  //设置线程池数量
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }

  //启动线程池
  void start(const ThreadInitCallback& cb = ThreadInitCallback());

  // valid after calling start()
  /// round-robin
  //获得下个loop以顺序轮流的形式
  EventLoop* getNextLoop();

  /// with the same hash code, it will always return the same EventLoop
  //根据hash值获得对应的loop
  EventLoop* getLoopForHash(size_t hashCode);

  //获得所有的loop
  std::vector<EventLoop*> getAllLoops();

  bool started() const
  { return started_; }

  const string& name() const
  { return name_; }

 private:

  EventLoop* baseLoop_;      //只用于接受连接的eventloop
  string name_;              
  bool started_;
  int numThreads_;          //线程池中线程的数量
  int next_;                //下个loop的关键字
  boost::ptr_vector<EventLoopThread> threads_;  //线程容器
  std::vector<EventLoop*> loops_;               //loop容器
};

}
}

#endif  // MUDUO_NET_EVENTLOOPTHREADPOOL_H
