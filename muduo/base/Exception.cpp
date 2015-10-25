/*************************************************************************
    > File Name: Exception.cpp
    > Author: MiaoShuai
    > Mail: 945970809@qq.com 
    > Created Time: 2015年10月25日 星期日 09时19分53秒
 ************************************************************************/
#include <muduo/base/Exception.h>

#include "Exception.h"

#include <execinfo.h>			//获取堆栈信息函数的头文件
#include <stdlib.h>

using namespace muduo;

Exception::Exception(const char * msg)
	:message_(msg)
{
	//获取当前运行的堆栈内容
	fillStackTrace();
}

Exception::Exception(const string &msg)
	:message_(msg)
{
	fillStackTrace();
}

Exception::Exception()throw()
{

}

const char *Exception::what()const throw()
{
	return message_.c_str();
}

const char *Exception::stackTrace()const throw()
{
	return stack_.c_str();
}

void Exception::fillStackTrace()
{
	//200个字符数组
	const int len = 200;
	void *buffer[len];   ///指针链表
	int nptrs = ::backtrace(buffer,len);  //将堆栈内容保存到buffer中
	char ** strings = ::backtrace_symbols_fd(buffer,nptrs);  //用char **指针接收刚才的内容

	if(strings)
	{
		for(int i = 0; i < nptrs; i++)
		{
			stack_.append(strings[i]);
			stack_.push_back('\n');
		}
		free(strings);
	}

}
