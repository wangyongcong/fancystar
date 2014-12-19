#include "winpch.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>
#include "wyc/util/strutil.h"
#include "wyc/util/util.h"
#include "console/xconsole.h"
#include "console/xlog.h"

#pragma warning (disable:4100)

namespace wyc
{

static int cmd_help (const char *argv[], unsigned argn)
{
	std::stringstream ss;
	ss<<"xconsole v"<<XCONSOLE_VERSION<<" by WYC"<<std::endl;
	ss<<"help: 显示帮助"<<std::endl;
	ss<<"print [string]: 打印文本"<<std::endl;
	ss<<"hash [string]: 显示字符串的散列值并复制到剪贴板"<<std::endl;
	ss<<"cls: 清空屏幕"<<std::endl;
	ss<<"exit: 退出控制台"<<std::endl;
	wyc_log(LOG_SYS,ss.str().c_str());
	return 0;
}

static int cmd_print (const char *argv[], unsigned argn)
{
	std::stringstream ss;
	for(unsigned i=1; i<argn; ++i) 
		ss<<argv[i]<<" ";
	wyc_log(LOG_NORMAL,ss.str().c_str());
	return 0;
}

static bool set_clipboard_data(HWND hwnd, const std::string &text)
{
	if(!::OpenClipboard(hwnd) || !::EmptyClipboard())
		return false;
	size_t len=text.size();
	HGLOBAL hglbCopy=::GlobalAlloc(GMEM_MOVEABLE,len+1); 
	if (hglbCopy == NULL) { 
		::CloseClipboard(); 
		return false; 
	} 
	char* pdst=(char*)::GlobalLock(hglbCopy); 
	memcpy(pdst,text.c_str(),len); 
	pdst[len]=0; 
	::GlobalUnlock(hglbCopy);
	::SetClipboardData(CF_TEXT,hglbCopy);
	::CloseClipboard();
	return true;
}

static int cmd_hash (const char *argv[], unsigned argn)
{
	if(argn<2) 
		return 1;
	std::string strhex;
	unsigned code=strhash(argv[1]);
	wyc::hex2str(strhex,code);
	strhex+=" // ";
	strhex+=argv[1];
	if(xconsole::has_console()) {
		set_clipboard_data(xconsole::singleton().get_window(),strhex);
	}
	wyc_log(LOG_NORMAL,"hash: %s",strhex);
	return 0;
}

static int cmd_exit (const char *argv[], unsigned argn)
{
	return 0;
}

static int cmd_cls (const char *argv[], unsigned argn)
{
	system("cls");
	return 0;
}

struct command_slot
{
	const char *cmd;
	CommandHandler handler;
};

int initialize_console_command()
{
	command_slot default_commands[]=
	{
		{"help",&cmd_help},
		{"print",&cmd_print},
		{"hash",&cmd_hash},
		{"exit",&cmd_exit},
		{"cls",&cmd_cls},
		{0,0},
	};
	command_slot *iter=default_commands;
	while(iter->cmd!=0) {
		assert(iter->handler);
		wyc_regcmd(iter->cmd,iter->handler);
		++iter;
	}
	return sizeof(default_commands)/sizeof(command_slot)-1;
}

}; // namespace wyc

