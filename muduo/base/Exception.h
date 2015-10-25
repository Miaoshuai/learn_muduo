/*************************************************************************
    > File Name: Exception.h
    > Author: MiaoShuai
    > Mail: 945970809@qq.com 
    > Created Time: 2015年10月25日 星期日 09时08分08秒
 ************************************************************************/

#ifndef MUDUO_BASE_EXCEPTION_H
#define MUDUO_BASE_EXCEPTION_H


#include <muduo/base/Types.h>
#include "Types.h"


#include <exception>

namespace muduo
{
	class Exception : public std::exception
	{
		public:
			explicit Exception(const char *what);
			explicit Exception(const string &what);
			virtual	 ~Exception()thow();
			virtual  const char *what()const throw();
			const    char *stackTrace()const throw();	//堆栈跟踪
		private:
			//获取当前位置堆栈内容
			void fillStackTrace();

			string message_;
			string stack_;		//保存堆栈信息
	};
}

#endif
