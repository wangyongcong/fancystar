#include "fscorepch.h"
#include <Mmsystem.h>
#include "wyc/mem/memtracer.h"
#include "wyc/util/util.h"
#include "wyc/util/fpath.h"
#include "wyc/obj/object.h"
#include "wyc/render/renderer.h"
#include "wyc/render/font.h"
#include "wyc/game/game.h"

namespace wyc
{

xthread_local xgame::ms_tlsGameContext;
wyc::xcritical_section xgame::ms_globalLock;
unsigned xgame::ms_processState;
xgame::xgame_window_map_t xgame::ms_gameWinMap;

bool xgame::init_game_process()
{
	ms_globalLock.lock();
	if(0!=ms_processState) {
		ms_globalLock.unlock();
		return true;
	}
	ms_processState=1;
	ms_tlsGameContext.alloc();
	xobject::get_context=&xgame::get_object_context;
	ms_globalLock.unlock();
	return true;
}

void xgame::exit_game_process()
{
	ms_globalLock.lock();
	if(1!=ms_processState) {
		ms_globalLock.unlock();
		return;
	}
	ms_processState=2;
	ms_tlsGameContext.free();
	ms_globalLock.unlock();
	wyc::xmemtracer::singleton().report();
}

bool xgame::game_process_ready()
{
	return 1==ms_processState;
}

xobject_context* xgame::get_object_context()
{
	xgame* pGame=(xgame*)ms_tlsGameContext.get_data();
	assert(pGame);
	return &pGame->m_objctx;
}

xgame::xgame() 
{
	m_hInstance=NULL;
	m_hMainWnd=NULL;
	m_hAccel=NULL;
	m_hTargetWnd=NULL;
	m_hGameThread=NULL;
	m_tidGame=0;
	m_hWorkerThread=NULL;
	m_tidWorker=0;

	m_app_name=L"Fancy Star Demo";
	m_res_path="data";
	m_gameState=XWIN_LOCK_FRAME;
	m_nWidth=m_nHeight=0;

	m_scheduler=0;
	m_worker_awaked=false;
	m_end_task=false;
	m_shutdown_worker=false;

	m_logic_period=1.0/60;
}

xgame::~xgame() 
{
}

void xgame::init_work_path() 
{
	wchar_t path[MAX_PATH];
	unsigned len=::GetModuleFileName(NULL,path,MAX_PATH);
	if(len>0) {
		m_app_file=path;
		wchar_t *pend=wcsrchr(path,L'\\');
		if(pend) 
			*pend=0;
		if(!::SetCurrentDirectory(path)) {
			wyc_warn("无法定位程序目录: %S",path);
		}
		m_work_path=path;
	}
	DWORD bufsz=MAX_PATH;
	HANDLE hToken;
	if(::OpenProcessToken(::GetCurrentProcess(),TOKEN_QUERY,&hToken)) {
		if(::GetUserProfileDirectory(hToken,path,&bufsz)) {
			m_home_path=path;
		}
		CloseHandle(hToken);
	}
}

HWND xgame::create_game_window(HINSTANCE hInstance, const wchar_t* strAppTitle, int width, int height, bool bFullScreen)
{
	const wchar_t* strClsName=L"FSGameWindow";
	WNDCLASSEX wndcls;
	if(!GetClassInfoEx(hInstance,strClsName,&wndcls)) {
		wndcls.cbSize=sizeof(WNDCLASSEX);
		wndcls.style=CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wndcls.hInstance=hInstance;
		wndcls.lpfnWndProc=WNDPROC(&xgame::window_proc);
		wndcls.cbClsExtra=0;
		wndcls.cbWndExtra=0;
		wndcls.hCursor=LoadCursor(NULL,IDC_ARROW);
		wndcls.hIcon=(HICON)::LoadImage(NULL,L"data/favicon32.ico",IMAGE_ICON,0,0,LR_LOADFROMFILE);
		wndcls.hIconSm=(HICON)::LoadImage(NULL,L"data/favicon16.ico",IMAGE_ICON,0,0,LR_LOADFROMFILE);
		wndcls.hbrBackground=NULL;
		wndcls.lpszMenuName=NULL;
		wndcls.lpszClassName=strClsName;
		if(!RegisterClassEx(&wndcls)) {
			wyc_error("Failed to register window class");
			return NULL;
		}
	}
	// 窗口属性
	DWORD dwWndStyle=WS_CLIPCHILDREN;
	DWORD dwWndStyleEx=0;
	// 窗口尺寸
	RECT rectClient;
	rectClient.left=0;
	rectClient.top=0;
	rectClient.right=width;
	rectClient.bottom=height;
	if(bFullScreen) {
		DEVMODE dm;
		memset(&dm,0,sizeof(DEVMODE));
		dm.dmSize=sizeof(DEVMODE);
		dm.dmPelsWidth=width;
		dm.dmPelsHeight=height;
		dm.dmBitsPerPel=32;
		dm.dmFields=DM_PELSWIDTH|DM_PELSHEIGHT|DM_BITSPERPEL;
		if(::ChangeDisplaySettings(&dm,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL) {
			if(IDYES!=MessageBox(0,L"全屏模式设置失败，是否改用窗口模式？",L"FancyStar",MB_YESNO|MB_ICONEXCLAMATION)) 
				return NULL;
			bFullScreen=false;
		}
	}
	int winWidth,winHeight,dxClient,dyClient,posx,posy;
	if(bFullScreen) { // 全屏模式
		dwWndStyle|=WS_POPUP;
		dwWndStyleEx|=WS_EX_APPWINDOW;
		ShowCursor(FALSE);
		AdjustWindowRectEx(&rectClient,dwWndStyle,false,dwWndStyleEx);
		winWidth=rectClient.right-rectClient.left;
		winHeight=rectClient.bottom-rectClient.top;
		dxClient=0;
		dyClient=0;
		posx=0;
		posy=0;
	}
	else { // 窗口模式
		dwWndStyle|=WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_POPUP;
		// 调整窗口尺寸
		AdjustWindowRectEx(&rectClient,dwWndStyle,false,dwWndStyleEx);
		winWidth=rectClient.right-rectClient.left;
		winHeight=rectClient.bottom-rectClient.top;
		// 计算客户区左上角的偏移
		dxClient=(winWidth-width)>>1;
		dyClient=winHeight-height-dxClient;
		// 居中窗口
		RECT rectDesk;
		GetWindowRect(GetDesktopWindow(),&rectDesk);
		int wDesk=rectDesk.right-rectDesk.left;
		int hDesk=rectDesk.bottom-rectDesk.top;
		posx=(wDesk-winWidth)>>1;
		posy=(hDesk-winHeight)>>1;
	}
	// 创建主窗口
	return CreateWindowEx(dwWndStyleEx, strClsName, strAppTitle, dwWndStyle, 
		posx, posy, winWidth, winHeight, NULL, NULL, hInstance, NULL);
}

HWND xgame::create_target_window(HINSTANCE hInstance, HWND hParent, const wchar_t *name, int x, int y, unsigned width, unsigned height)
{
	assert(hInstance);
	const wchar_t *className=L"FSOpenGLTargetWindow";
	WNDCLASSEX wndcls;
	if(!GetClassInfoEx(hInstance,className,&wndcls)) {
		wndcls.cbSize=sizeof(WNDCLASSEX);
		wndcls.style=CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wndcls.hInstance=hInstance;
		wndcls.lpfnWndProc=WNDPROC(&DefWindowProc);
		wndcls.cbClsExtra=0;
		wndcls.cbWndExtra=0;
		wndcls.hCursor=LoadCursor(NULL,IDC_ARROW);
		wndcls.hIcon=NULL;
		wndcls.hIconSm=NULL;
		wndcls.hbrBackground=NULL;
		wndcls.lpszMenuName=NULL;
		wndcls.lpszClassName=className;
		if(!RegisterClassEx(&wndcls)) 
			return NULL;
	}
	DWORD style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	if(NULL!=hParent) 
		style |= WS_CHILDWINDOW;
	return CreateWindowEx(0, className, name, style, 
		x, y, width, height, hParent, NULL, hInstance, NULL);
}

bool xgame::add_game_window(HWND hwnd, xgame *pgame)
{
	ms_globalLock.lock();
	size_t cnt=ms_gameWinMap.size();
	for(size_t i=0; i<cnt; ++i) {
		if(ms_gameWinMap[i].first==hwnd) {
			ms_globalLock.unlock();
			return true;
		}
	}
	ms_gameWinMap.push_back(std::pair<HWND,xgame*>(hwnd,pgame));
	ms_globalLock.unlock();
	return true;
}

void xgame::remove_game_window(HWND hwnd)
{
	ms_globalLock.lock();
	size_t cnt=ms_gameWinMap.size(), last;
	for(size_t i=0; i<cnt; ++i) {
		if(ms_gameWinMap[i].first==hwnd) {
			last=cnt-1;
			if(i!=last) 
				ms_gameWinMap[i]=ms_gameWinMap[last];
			ms_gameWinMap.pop_back();
			break;
		}
	}
	ms_globalLock.unlock();
}

xgame* xgame::get_game_instance(HWND hwnd)
{
	xgame *pgame=0;
	ms_globalLock.lock();
	size_t cnt=ms_gameWinMap.size();
	for(size_t i=0; i<cnt; ++i) {
		if(ms_gameWinMap[i].first==hwnd) {
			pgame=ms_gameWinMap[i].second;
			break;
		}
	}
	ms_globalLock.unlock();
	return pgame;
}

bool xgame::create(HINSTANCE hInstance, HWND hMainWnd, const wchar_t* strAppTitle, int x, int y, int width, int height, bool bFullScreen)
{
	if(!game_process_ready())
		return false;
	init_work_path();
	m_hInstance=hInstance;
	if(strAppTitle)
		m_app_name=strAppTitle;
	if(NULL==hMainWnd) {
		// 创建主窗口
		m_hMainWnd=xgame::create_game_window(hInstance,m_app_name.c_str(),width,height,bFullScreen);
		if(NULL==m_hMainWnd) {
			wyc_error("Failed to create main game window");
			MessageBox(0,L"应用程序初始化失败，正在关闭",L"FancyStar",MB_OK|MB_ICONSTOP);
			return false;
		}
		xgame::add_game_window(m_hMainWnd,this);
	}
	else {
		m_hMainWnd=hMainWnd;
		add_state(m_gameState,XWIN_AS_CHILDREN);
	}
	set_fullscreen(bFullScreen);
	// 创建主渲染窗口
	if(0==width || 0==height) {
		RECT rectClient;
		GetClientRect(m_hMainWnd,&rectClient);
		if(0==width)
			width=rectClient.right-rectClient.left;
		if(0==height)
			height=rectClient.bottom-rectClient.top;
	}
	m_nWidth=width;
	m_nHeight=height;
	m_hTargetWnd=xgame::create_target_window(m_hInstance,m_hMainWnd,L"MainTargetWindow",0,0,m_nWidth,m_nHeight);
	if(NULL==m_hTargetWnd) {
		wyc_error("Failed to create target window");
		return false;
	}
	// 不响应输入消息,纯粹作显示用
	EnableWindow(m_hTargetWnd,FALSE);
	ShowWindow(m_hTargetWnd,SW_NORMAL);
	// 初始化渲染器
	if(!xrenderer::check_driver()) {
		// 这里必须加锁, 因为多个线程可能会同时到达这里
		ms_globalLock.lock();
		if(!xrenderer::check_driver()) {
			HWND hTmpWnd=create_target_window(hInstance,m_hMainWnd,L"TempWindow",0,0,m_nWidth,m_nHeight);
			if(NULL==hTmpWnd) {
				ms_globalLock.unlock();
				wyc_error("Failed to create temp window");
				return false;
			}
			if(!xrenderer::initialize_driver(hTmpWnd,"")) {
				DestroyWindow(hTmpWnd);
				ms_globalLock.unlock();
				MessageBox(0,L"OpenGL驱动程序初始化失败，正在关闭",L"FancyStar",MB_OK|MB_ICONSTOP);
				return false;
			}
			DestroyWindow(hTmpWnd);
		}
		ms_globalLock.unlock();
		assert(xrenderer::check_driver());
	}
	// 创建渲染器
	m_renderer=xrenderer::create_renderer(m_hTargetWnd);
	if(!m_renderer) {
		MessageBox(0,L"渲染器创建失败，正在关闭",L"FancyStar",MB_OK|MB_ICONSTOP);
		return false;
	}
	std::string shader_path = m_res_path;
	shader_path += "/shader";
	m_renderer->set_shader_path(shader_path);
	// 创建游戏逻辑线程
	if(!init_game_thread()) {
		MessageBox(0,L"应用程序初始化失败，正在关闭",L"FancyStar",MB_OK|MB_ICONSTOP);
		return false;
	}
	ShowWindow(m_hMainWnd,SW_NORMAL);
	return true;
}

int xgame::run()
{
	wyc_print("game start...");
	//------------------------------------------------
	// 分离游戏主线程
	if(have_state(m_gameState,XWIN_SEPARATED_GAME_THREAD)) {
		ResumeThread(m_hGameThread); 
		if(!is_child()) {
			MSG msg;
			while(GetMessage(&msg,NULL,0,0))
			{
				if(!TranslateAccelerator(m_hMainWnd,m_hAccel,&msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			exit_game_thread();
			xgame::remove_game_window(m_hMainWnd);
		}
		return 0;
	}
	//------------------------------------------------
	// 共享游戏主线程
	if(!init_game())
	{
		wyc_error("game init failed");
		return 0;
	}
	MSG msg;
	while(true) {
		if(::PeekMessage(&msg,NULL,0,0,PM_REMOVE|PM_QS_INPUT|PM_QS_POSTMESSAGE))
		{
			if(msg.message == WM_QUIT) {
				add_state(m_gameState,XWIN_QUIT);
				break;
			}
			if(!TranslateAccelerator(m_hMainWnd,m_hAccel,&msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else 
		{
			m_clock.tick();
			if(m_logic_timer.elapsed_time()>=m_logic_period)
				process_logic();
			Sleep(1);
		}
	}
	quit_game();
	return 0;
}

bool xgame::init_game()
{
	// adjust timing precision
	if(MMSYSERR_NOERROR!=timeBeginPeriod(1))
	{
		wyc_warn("can't adjust timing period");
	}

	ms_tlsGameContext.set_data(this);
	if(!m_renderer->initialize()) {
		wyc_error("xgame::init_game: fail to create renderer");
		return false;
	}
	xsimple_scheduler *sh = new xsimple_scheduler();
	sh->set_worker(&m_worker);
	if(!sh->connect()) {
		wyc_error("xgame::init_game: can't connect to worker server");
		return false;
	}
	m_scheduler = sh;

	// init resource servers
	// because render resource must be created in the same thread as OpenGL context,
	// we need two servers: one for render resource, the other for common resource

	// TODO: cache size is vary in different app.
	// It should be loaded from config file

	// resource server
	if(!m_ressvr.create_group("xtexture",32)) {
		wyc_error("xgame::init_game: fail to create texture cache");
		return false;
	}
	if(!m_ressvr.create_group("xvertex_batch",32)) {
		wyc_error("xgame::init_game: fail to create batch cache");
		return false;
	}
	if(!m_ressvr.create_group("xfont",32)) {
		wyc_error("xgame::init_game: fail to create font cache");
		return false;
	}

	// custom initialization
	if(!on_game_init())
		return false;

	// reset clock
	m_clock.add_timer(&m_logic_timer);
	m_clock.add_timer(&m_metric_timer);
	m_clock.reset();

	return true;
}

void close_thread_impl(const char *name, HANDLE thread, unsigned time_out)
{
	assert(NULL!=thread);
	wyc_print("closing %s thread...",name);
	DWORD ret=WaitForSingleObject(thread,time_out);
	if(WAIT_OBJECT_0!=ret) {
		wyc_warn("%s thread close time out",name);
		TerminateThread(thread,0);
	}
	CloseHandle(thread);
	wyc_print("%s thread closed",name);
}

void xgame::quit_game() 
{
	on_game_exit();
	if(NULL!=m_hWorkerThread) {
		m_shutdown_worker.store(true,std::memory_order_release);
		ResumeThread(m_hWorkerThread);
		close_thread_impl("worker server",m_hWorkerThread,INFINITE);
		m_hWorkerThread=NULL;
		m_tidWorker=0;
	}
	if(m_scheduler) {
		m_scheduler->disconnect();
		delete m_scheduler;
		m_scheduler=0;
	}
	m_worker.shut_down();
	m_ressvr.shut_down();
	m_renderer->terminate();
	m_renderer=0;
	m_objctx.clear();

	// reset timing precision
	timeEndPeriod(1);
}

bool xgame::init_game_thread()
{	
	if(have_state(m_gameState,XWIN_SEPARATED_GAME_THREAD)) {
		assert(NULL==m_hGameThread);
		m_hGameThread=(HANDLE)_beginthreadex(NULL,0,xgame::game_proc,this,CREATE_SUSPENDED,&m_tidGame);
		if(NULL==m_hGameThread) {
			wyc_error("Failed to create game thread");
			return false;
		}
	}
	assert(NULL==m_hWorkerThread);
	m_hWorkerThread=(HANDLE)_beginthreadex(NULL,0,xgame::game_worker_proc,this,CREATE_SUSPENDED,&m_tidWorker);
	if(NULL==m_hWorkerThread) {
		wyc_error("Failed to create resource server thread");
		return false;
	}
	return true;
}

void xgame::exit_game_thread() 
{
	wyc_print("game is closing...");
	add_state(m_gameState,XWIN_QUIT);
	// shut down game thread
	if (NULL!=m_hGameThread) {
		close_thread_impl("main game",m_hGameThread,INFINITE);
		m_hGameThread=NULL;
		m_tidGame=0;
	}
	wyc_print("game exit");
}

void xgame::capture_mouse()
{
	if(have_state(m_gameState,XWIN_CAPTURED_MOUSE))
		return;
	add_state(m_gameState,XWIN_CAPTURED_MOUSE);
	SetCapture(m_hMainWnd);
}

void xgame::release_mouse()
{
	if(have_state(m_gameState,XWIN_CAPTURED_MOUSE))
	{
		ReleaseCapture();
		remove_state(m_gameState,XWIN_CAPTURED_MOUSE);
	}
}

void xgame::activate_window()
{
}

void xgame::deactivate_window()
{
	if(have_state(m_gameState,XWIN_CAPTURED_MOUSE))
	{
		ReleaseCapture();
		remove_state(m_gameState,XWIN_CAPTURED_MOUSE);
		POINT cursor_pos;
		::GetCursorPos(&cursor_pos);
		if(GetKeyState(VK_LBUTTON) & 0x80)
			input().mouse_button(EV_LB_UP,short(cursor_pos.x),short(cursor_pos.y),0);
		if(GetKeyState(VK_RBUTTON) & 0x80)
			input().mouse_button(EV_RB_UP,short(cursor_pos.x),short(cursor_pos.y),0);
		if(GetKeyState(VK_MBUTTON) & 0x80)
			input().mouse_button(EV_MB_UP,short(cursor_pos.x),short(cursor_pos.y),0);
	}
}

void xgame::set_resource_path(const char *path) 
{
	m_res_path = path;
	if(m_res_path.size()) {
		char c = m_res_path[m_res_path.size()-1];
		if('/'==c || '\\'==c)
			m_res_path.erase(m_res_path.size()-1);
	}
}

void xgame::get_resource_path(std::string &file_path) const {
	std::string path;
	path.reserve(file_path.size()+m_res_path.size()+1);
	path+=m_res_path;
	if(file_path.size() && file_path[0]!='/' && file_path[0]!='\\')
		path+="/";
	path+=file_path;
	file_path=path;
}

} // namespace wyc


