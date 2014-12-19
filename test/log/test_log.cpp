#include <iostream>
#define WYC_LOG
#include "wyc/util/util.h"

#pragma comment(lib,"fslog_d.lib")

using namespace wyc;

int main(int, char **)
{
	// if ansi encoding is used
	// must set locale first
	setlocale(LC_ALL,"chs");
	wyc_log_init("test_log","../../../tmp",LOG_NORMAL,false);
	
	wyc_print("user info");
	wyc_sys("system info");
	wyc_warn("warning!");
	wyc_error("this is an error");
	wyc_fatal("fatal error");
	wyc_logw(LOG_NORMAL,L"普通信息");
	wyc_logw(LOG_SYS,L"系统信息");
	wyc_logw(LOG_WARN,L"警告信息");
	wyc_logw(LOG_ERROR,L"错误信息");
	wyc_logw(LOG_FATAL,L"严重错误");

	wyc_log_close();

	printf("Press [Enter] to continue\n");
	getchar();
	return 0;
}

