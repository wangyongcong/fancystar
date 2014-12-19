#include <cstdlib>
#include <cstdio>
#include <iostream>
#include "wyc/util/util.h"
#include "picloud/picloud_client.h"

#ifdef _DEBUG
	#pragma comment(lib,"fsutil_d.lib")
	#pragma comment(lib,"fslog_d.lib")
#else
	#pragma comment(lib,"fsutil.lib")
	#pragma comment(lib,"fslog.lib")
#endif

using namespace wyc;

int main(int, char**)
{
	wyc_log_init("picloud.log","logs",LOG_NORMAL,false);
	initialize_internet_connection(L"Fancystar");

	bool is_done=false;
	std::string cmd;

	xpicloud_client *client=new xpicloud_client();
	client->connect();
	while(!is_done) {
		std::cin>>cmd;
		if(cmd=="exit")
			break;
	}
	cleanup_internet_connection();
	
	printf("Press [Enter] to continue...");
	getchar();
	return 0;
}


