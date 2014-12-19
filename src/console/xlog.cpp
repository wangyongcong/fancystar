#include "winpch.h"
#include "console/xlog.h"
#include "console/xconsole.h"
#include "wyc/util/hash.h"
#include "wyc/util/util.h"

#ifdef _DEBUG
	#pragma comment (lib, "fsutil_d.lib")
#else
	#pragma comment (lib, "fsutil.lib")
#endif

namespace wyc 
{

int initialize_console_command();

struct xlog
{
	CRITICAL_SECTION m_outputLock;
	xdict m_cmdmap;
	bool create() 
	{
		if(!InitializeCriticalSectionAndSpinCount(&m_outputLock,4000))
			return false;
		initialize_console_command();
		return true;
	}
	void destroy()
	{
		DeleteCriticalSection(&m_outputLock);
	}
	inline void lock()
	{
		EnterCriticalSection(&m_outputLock);
	}
	inline void unlock()
	{
		LeaveCriticalSection(&m_outputLock);
	}
};

}; // namespace wyc

using wyc::xlog;
using wyc::xconsole;

xlog s_log;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, void* lpReserved)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		s_log.create();
		xconsole::create_console();
		break;

	case DLL_PROCESS_DETACH:
		s_log.destroy();
		xconsole::free_console();
		break;
	}
	return TRUE;
}

#define DEFAULT_LOG_BUFFER 1024

void wyc_log_init(const char *name, const char *log_dir, int log_level)
{
	// redirect standard output
	if(xconsole::has_console()) {
#pragma warning(push)
#pragma warning(disable:4996)
	//	freopen("CONIN$","r+t",stdin);
		freopen("CONOUT$","w+t",stdout);
		freopen("CONOUT$","w+t",stderr);
#pragma warning(pop)
	}
}

void wyc_log(int level, const char *format, ...)
{
	char buff[DEFAULT_LOG_BUFFER], *palloc=0;
	const char *pinfo;
	va_list arglist;
	va_start(arglist,format);
	int cnt=::vsprintf_s(buff,DEFAULT_LOG_BUFFER,format,arglist);
	if(cnt+2<=DEFAULT_LOG_BUFFER) {
		buff[cnt++]='\n';
		buff[cnt]=0;
		pinfo=buff;
	}
	else {
		cnt=::_vscprintf(format,arglist);
		palloc=new char[cnt+2];
		::vsprintf_s(palloc,cnt,format,arglist);
		palloc[cnt++]='\n';
		palloc[cnt]=0;
		pinfo=palloc;
	}
	va_end(arglist);
	
	if(xconsole::has_console()) {
		xconsole &con=xconsole::singleton();
		s_log.lock();
		con.set_flag(level);
		con.write(pinfo,cnt);
		s_log.unlock();
	}
	else printf(pinfo);
	if(palloc) {
		delete [] palloc;
	}
}

void wyc_logw(int level, const wchar_t *format, ...)
{
	wchar_t buff[DEFAULT_LOG_BUFFER], *palloc=0;
	const wchar_t *pinfo;
	va_list arglist;
	va_start(arglist,format);
	int cnt=::vswprintf_s(buff,DEFAULT_LOG_BUFFER,format,arglist);
	if(cnt+2<=DEFAULT_LOG_BUFFER) {
		buff[cnt++]=L'\n';
		buff[cnt]=0;
		pinfo=buff;
	}
	else {
		cnt=::_vscwprintf(format,arglist);
		palloc=new wchar_t[cnt+2];
		::vswprintf_s(palloc,cnt,format,arglist);
		palloc[cnt++]=L'\n';
		palloc[cnt]=0;
		pinfo=palloc;
	}
	va_end(arglist);

	if(xconsole::has_console()) {
		xconsole &con=xconsole::singleton();
		s_log.lock();
		con.set_flag(level);
		con.write(pinfo,cnt);
		s_log.unlock();
	}
	else wprintf(pinfo);
	if(palloc) {
		delete [] palloc;
	}
}

bool wyc_regcmd(const char *cmd, CommandHandler handler)
{
	unsigned cmdid=wyc::strhash(cmd);
	return s_log.m_cmdmap.add(cmdid,handler);
}

#pragma warning (push)
#pragma warning (disable : 4127) // warning C4127: 条件表达式是常量
#pragma warning (disable : 4996) // warning C4996: 'strtok': This function or variable may be unsafe.

#define MAX_COMMAND_LINE 255

unsigned int input_process(void*)
{
	char strbuff[MAX_COMMAND_LINE+1];
	const char *delimit=" \t\n";
	const char *argv[32], *token;
	unsigned argn;
	void* handler;
	while(true) {
		int c=getchar();
		char *iter=strbuff, *end=iter+MAX_COMMAND_LINE;
		while(iter!=end && c!=EOF && c!='\n') {
			*iter++=char(c);
			c=getchar();
		}
		*iter=0;
		argn=0;
		token=::strtok(strbuff,delimit);
		while(token!=NULL && argn<32) {
			argv[argn++]=token;
			token=::strtok(NULL,delimit);
		}
		if(argn<1)
			continue;
		handler = s_log.m_cmdmap.get(wyc::strhash(argv[0]));
		if(!handler)
			continue;
		((CommandHandler)handler)(argv,argn);
	}
	return 0;
}

#pragma warning (pop)

