/*************************************************************************
    > File Name: CurrentThread.h
    > Author: MiaoShuai
    > Mail: 945970809@qq.com 
    > Created Time: 2015年10月24日 星期六 08时53分47秒
 ************************************************************************/
#ifndef MUDUO_BASE_CURRENTTHREAD_H
#define MUDUO_BASE_CURRENTTHREAD_H

#include<stdint.h>

namespace muduo
{
namespace CurrentThread
{
	//使用__thread定义自己的线程属性
	extern __thread int  t_cachedTid;        //当前线程ID
	extern __thread char t_tidString[32];    //线程ID的字符串，方便输出日志
	extern __thread int  t_tidStringLength;  //字符串的长度
	extern __thread const char *t_threadName;//线程的名字

	//获取当前线程ID
	void cacheTid();

	inline int tid()
	{
		//内建函数的一个优化，我们也可以用unlikely替换，分支预测
		if(__builtin_expect(t_cachedTid == 0,0))
		{
			cacheTid();
		}

		return t_cachedTid;
	}

	//线程ID的字符串，方便写日志
	inline const char* tidString()
	{
		return t_tidString;
	}
	
	//长度
	inline int tidStringLength()
	{
		return t_tidStringLength;
	}

	//名字
	inline const char *threadName()
	{
		return t_threadName;
	}

	//当前线程是否为主线程
	bool isMainThread();
	
	//睡眠时长
	void sleepUsec(int64_t usec);
}
}
