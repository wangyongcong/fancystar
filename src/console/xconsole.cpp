#include "winpch.h"
#include "xconsole.h"
#include <cstdio>
#include <string>

extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow ();

namespace wyc
{

#define MONITOR_DEFAULT_POSX	10
#define MONITOR_DEFAULT_POSY	10
#define MONITOR_DEFAULT_WIDTH	670
#define MONITOR_DEFAULT_HEIGH	300
#define MONITOR_DEFAULT_COL		80
#define MONITOR_DEFAULT_ROW		300
#define	MONITOR_BUFF_INCREASE	128
#define MONITOR_DEF_BUFFSIZE	128

xconsole* xconsole::ms_pConsole=0;

#pragma warning (push)
#pragma warning (disable : 4996) // warning C4996: 'freopen': This function or variable may be unsafe.

bool xconsole::create_console(unsigned window_width, unsigned window_height, bool redirect_stdio) {
	if(ms_pConsole) 
		return true;
	if(!AllocConsole())
		return false;
	HANDLE hOut=::GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE hIn=::GetStdHandle(STD_INPUT_HANDLE);
	HWND hWnd=::GetConsoleWindow();
	if(hOut==NULL || hIn==NULL || hWnd==NULL) 
		return false;
	SetConsoleMode(hOut, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
	SetConsoleTitle(L"xconsole");

	ms_pConsole=new xconsole;
	ms_pConsole->m_hWnd=hWnd;
	ms_pConsole->m_hOut=hOut;
	ms_pConsole->m_hInput=hIn;
	if(window_width<1)
		window_width=MONITOR_DEFAULT_WIDTH;
	if(window_height<1)
		window_height=MONITOR_DEFAULT_HEIGH;
	ms_pConsole->set_posize(MONITOR_DEFAULT_POSX,MONITOR_DEFAULT_POSY,window_width,window_height);

	if(redirect_stdio) {
		// 重定向 stdin 和 stdout
		freopen("CONIN$","r+t",stdin);
		freopen("CONOUT$","w+t",stdout);
	}
	return true;
}

#pragma warning (pop)

xconsole::xconsole()
{
	m_hWnd=NULL;
	m_hOut=NULL;
	m_hInput=NULL;
	m_curFlag=0;
}

xconsole::~xconsole()
{
	// 控制台跟应用程序的生命周期是一致的，控制台关闭的时候程序就会退出，反之亦然
	// 所以控制台窗口和输入线程留给Windows来清理
	// 如果在退出的时候调用：FreeConsole() 在Win7下会异常
}

static WORD CONSOLE_COLOR[6]={
	FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY,
	FOREGROUND_GREEN|FOREGROUND_INTENSITY,
	FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_INTENSITY,
	FOREGROUND_RED|FOREGROUND_INTENSITY,
	FOREGROUND_GREEN|FOREGROUND_BLUE|FOREGROUND_INTENSITY,
	FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_INTENSITY,
};

void xconsole::set_flag(unsigned nType)
{
	if(NULL==m_hOut || m_curFlag==nType)
		return;
	m_curFlag=nType;
	assert(nType<5);
	::SetConsoleTextAttribute(m_hOut,CONSOLE_COLOR[m_curFlag]);
}

void xconsole::set_posize(int x, int y, int w, int h) 
{
	if(m_hWnd==NULL) return;
	::MoveWindow(m_hWnd,x,y,w,h,true);
	COORD cs;
	/*
	// 根据窗口大小计算缓冲区大小
	CONSOLE_FONT_INFO fi;
	::GetCurrentConsoleFont(m_hOut,FALSE,&fi);
	fi.dwFontSize=::GetConsoleFontSize(m_hOut,fi.nFont);
	cs.X=short(w/fi.dwFontSize.X);
	cs.Y=MONITOR_DEFAULT_ROW;
	*/
	cs.X=MONITOR_DEFAULT_COL;
	cs.Y=MONITOR_DEFAULT_ROW;
	::SetConsoleScreenBufferSize(m_hOut,cs);
}

void xconsole::free_console()
{
	assert(ms_pConsole);
	ms_pConsole=0;
	::FreeConsole();
}

} // namespace wyc

