/**	@file Windows GUI程序外挂控制台
	@version 1.0.1
	@author ycwang
	@date 2010-07-17
*/
#ifndef __HEADER_WYC_XCONSOLE
#define __HEADER_WYC_XCONSOLE

#include <cassert>

namespace wyc
{

/** @brief 外挂控制台，用于Windows GUI程序中输出调试信息，执行调试指令
	@details 每个应用程序只能创建一个全局的xconsole对象，通过singleton()可以获取该全局对象的引用\n
	当控制台创建的时候，同时会创建一个输入线程，等待和处理用户指令。目前可用的指令有：\n
	- <i>help</i>: 打印帮助信息
	- <i>print [string]</i>: 打印一行字符串
	- <i>hash [string]</i>: 输出字符串的Hash值，并复制到剪贴板
	- <i>hide</i>:	隐藏控制台
	- <i>cls</i>:	清屏
	- <i>exit</i>:	退出控制台
*/
class xconsole
{
	HWND m_hWnd;
	HANDLE m_hOut, m_hInput;
	DWORD m_curFlag;
	static xconsole *ms_pConsole;
public:
	/**	@brief 返回全局控制台的引用
		@details 如果控制台没有创建，将会抛出异常
	*/
	static xconsole& singleton();
	/// 控制台是否存在
	static bool has_console();
	/**	@brief 创建控制台
		@param window_width 窗口的宽度，设为0表示使用默认宽度
		@param window_height 窗口的高度，设为0表示使用默认宽度
		@param redirect_stdio 是否重定向C++标准输入输出流
		@return 成功返回true，失败返回false
	*/
	static bool create_console(unsigned window_width=0, unsigned window_height=0, bool redirect_stdio=true);
	/// 删除控制台
	static void free_console();

	//==========================================

	/**	@brief 设置控制台的位置和大小
		@param x 窗口x坐标
		@param y 窗口y坐标
		@param w 窗口宽度
		@param h 窗口高度
	*/
	void set_posize(int x, int y, int w, int h);
	/// 显示控制台
	void show();
	/// 隐藏控制台
	void hide();
	/// 取窗口句柄
	HWND get_window() const;
	/**	@brief 设置控制台输出格式
		@param flag 格式常量，取值如下：
			- 0 用户输出
			- 1 系统输出
			- 2 警告
			- 3 错误
	*/
	void set_flag(unsigned flag);
	/// 在控制台上打印一行消息
	unsigned write(const char *pstr, unsigned size);
	unsigned write(const wchar_t *pwstr, unsigned size);
private:
	xconsole();
	xconsole(const xconsole &console);
	xconsole& operator = (const xconsole &console);
	~xconsole();
};

#define XCONSOLE_VERSION "1.0.1"

inline xconsole& xconsole::singleton() {
	assert(ms_pConsole || !"wyc_console has not been created");
	return *ms_pConsole;
}

inline bool xconsole::has_console() {
	return ms_pConsole!=0;
}

inline HWND xconsole::get_window() const
{
	return ms_pConsole->m_hWnd;
}

inline void xconsole::show() {
	if(m_hWnd==NULL) return;
	::ShowWindow(m_hWnd,SW_SHOWNOACTIVATE);
}

inline void xconsole::hide() {
	if(m_hWnd==NULL) return;
	::ShowWindow(m_hWnd,SW_HIDE);
}

inline unsigned xconsole::write(const char *pstr, unsigned size)
{
	DWORD ret;
	::WriteConsoleA(m_hOut,pstr,size,&ret,0);
	return ret;
}

inline unsigned xconsole::write(const wchar_t *pwstr, unsigned size)
{
	DWORD ret;
	::WriteConsoleW(m_hOut,pwstr,size,&ret,0);
	return ret;
}


} // namespace wyc

#endif // end of __HEADER_WYC_XCONSOLE


