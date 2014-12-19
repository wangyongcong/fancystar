/**********************************************************\

  Auto-generated Fancystar.cpp

  This file contains the auto-generated main plugin object
  implementation for the Fancystar project

\**********************************************************/
#include "FancystarPCH.h"

#include "PluginWindowWin.h"
#include "FancystarAPI.h"
#include "Fancystar.h"

extern HINSTANCE gInstance;

std::wstring Fancystar::m_gamePath;

///////////////////////////////////////////////////////////////////////////////
/// @fn Fancystar::StaticInitialize()
///
/// @brief  Called from PluginFactory::globalPluginInitialize()
///
/// @see FB::FactoryBase::globalPluginInitialize
///////////////////////////////////////////////////////////////////////////////
void Fancystar::StaticInitialize()
{
    // Place one-time initialization stuff here; As of FireBreath 1.4 this should only
    // be called once per process
	wchar_t wsbuff[MAX_PATH];
	DWORD bufsz=MAX_PATH;
	HANDLE hToken;
	if(::OpenProcessToken(::GetCurrentProcess(),TOKEN_QUERY,&hToken)) {
		if(::GetUserProfileDirectory(hToken,wsbuff,&bufsz)) {
			m_gamePath=wsbuff;
			m_gamePath+=L"\\fancystar\\bin\\";
			SetDllDirectory(m_gamePath.c_str());
		}
		CloseHandle(hToken);
	}
	LoadLibrary(L"fsconsole.dll");
}

///////////////////////////////////////////////////////////////////////////////
/// @fn Fancystar::StaticInitialize()
///
/// @brief  Called from PluginFactory::globalPluginDeinitialize()
///
/// @see FB::FactoryBase::globalPluginDeinitialize
///////////////////////////////////////////////////////////////////////////////
void Fancystar::StaticDeinitialize()
{
    // Place one-time deinitialization stuff here. As of FireBreath 1.4 this should
    // always be called just before the plugin library is unloaded
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Fancystar constructor.  Note that your API is not available
///         at this point, nor the window.  For best results wait to use
///         the JSAPI object until the onPluginReady method is called
///////////////////////////////////////////////////////////////////////////////
Fancystar::Fancystar()
{
	m_hGameContext=0;
	m_hGameLoader=NULL;
	on_mouse_button=0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Fancystar destructor.
///////////////////////////////////////////////////////////////////////////////
Fancystar::~Fancystar()
{
    // This is optional, but if you reset m_api (the shared_ptr to your JSAPI
    // root object) and tell the host to free the retained JSAPI objects then
    // unless you are holding another shared_ptr reference to your JSAPI object
    // they will be released here.
    releaseRootJSAPI();
    m_host->freeRetainedObjects();
}

void Fancystar::onPluginReady()
{
    // When this is called, the BrowserHost is attached, the JSAPI object is
    // created, and we are ready to interact with the page and such.  The
    // PluginWindow may or may not have already fire the AttachedEvent at
    // this point.
}

void Fancystar::shutdown()
{
    // This will be called when it is time for the plugin to shut down;
    // any threads or anything else that may hold a shared_ptr to this
    // object should be released here so that this object can be safely
    // destroyed. This is the last point that shared_from_this and weak_ptr
    // references to this object will be valid
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Creates an instance of the JSAPI object that provides your main
///         Javascript interface.
///
/// Note that m_host is your BrowserHost and shared_ptr returns a
/// FB::PluginCorePtr, which can be used to provide a
/// boost::weak_ptr<Fancystar> for your JSAPI class.
///
/// Be very careful where you hold a shared_ptr to your plugin class from,
/// as it could prevent your plugin class from getting destroyed properly.
///////////////////////////////////////////////////////////////////////////////
FB::JSAPIPtr Fancystar::createJSAPI()
{
    // m_host is the BrowserHost
    return boost::make_shared<FancystarAPI>(FB::ptr_cast<Fancystar>(shared_from_this()), m_host);
}

bool Fancystar::onKeyDown(FB::KeyDownEvent *evt, FB::PluginWindow *)
{
	on_key_down(m_hGameContext,evt->m_os_key_code);
	return true;
}

bool Fancystar::onKeyUp(FB::KeyUpEvent *evt, FB::PluginWindow *)
{
	on_key_up(m_hGameContext,evt->m_os_key_code);
	return true;
}

bool Fancystar::onMouseDown(FB::MouseDownEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse down at: %d, %d\n", evt->m_x, evt->m_y);
	on_mouse_button(m_hGameContext,BUTTON_DOWN|evt->m_Btn,evt->m_x,evt->m_y);
    return true;
}

bool Fancystar::onMouseUp(FB::MouseUpEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse up at: %d, %d\n", evt->m_x, evt->m_y);
	on_mouse_button(m_hGameContext,evt->m_Btn,evt->m_x,evt->m_y);
	return true;
}

bool Fancystar::onMouseMove(FB::MouseMoveEvent *evt, FB::PluginWindow *)
{
    //printf("Mouse move at: %d, %d\n", evt->m_x, evt->m_y);
	on_mouse_move(m_hGameContext,evt->m_x,evt->m_y);
    return true;
}

bool Fancystar::onWindowAttached(FB::AttachedEvent *evt, FB::PluginWindow *pwin)
{
    // The window is attached; act appropriately
	return true;
}

bool Fancystar::onWindowDetached(FB::DetachedEvent *evt, FB::PluginWindow *)
{
	if(m_hGameLoader) {
		if(m_hGameContext) {
			typedef void (*pfnClearGame) (void*);
			pfnClearGame clear_game=(pfnClearGame)::GetProcAddress(m_hGameLoader,"clear_game");
			clear_game(m_hGameContext);
			m_hGameContext=0;
		}
		// free module 会导致 crash
	//	FreeLibrary(m_hGameLoader);
		m_hGameLoader=NULL;
	}
	return true;
}

bool Fancystar::onWindowRefresh(FB::RefreshEvent *evt, FB::PluginWindow *)
{
	return true;
}

bool Fancystar::load_game(const std::string &name)
{
	FB::PluginWindowWin *pWindowWin=dynamic_cast<FB::PluginWindowWin*>(GetWindow());
	if(pWindowWin==0) {
		::MessageBox(NULL,L"插件窗口错误",L"插件错误",MB_OK);
		return false;
	}
	HWND hWnd=pWindowWin->getHWND();
	if(hWnd==NULL) {
		::MessageBox(NULL,L"窗口句柄无效",L"插件错误",MB_OK);
		return false;
	}
	RECT rect;
	::GetClientRect(hWnd,&rect);
	int width=rect.right-rect.left, height=rect.bottom-rect.top;
	
	wchar_t wsbuff[255];
	size_t count=::mbstowcs(wsbuff,name.c_str(),255);
	if(count==-1 || count==255) {
		::MessageBox(NULL,L"非法游戏资源",L"插件错误",MB_OK);
		return false;
	}
	std::wstring gameFile=wsbuff;
	gameFile+=L".dll";
	m_hGameLoader=::LoadLibrary(gameFile.c_str());
	if(m_hGameLoader==NULL) {
		::MessageBox(NULL,L"无法读取游戏资源",L"插件错误",MB_OK);
		return false;
	}
	typedef void* (*pfnLoadGame) (HINSTANCE,HWND,wchar_t*,int,int,int,int);
	pfnLoadGame load_game=(pfnLoadGame)::GetProcAddress(m_hGameLoader,"load_game");
	if(load_game==NULL) {
		::MessageBox(NULL,L"Load game failed",L"插件错误",MB_OK);
		return false;
	}
	m_hGameContext=load_game(gInstance,hWnd,wsbuff,0,0,width,height);
	if(!m_hGameContext) {
		::MessageBox(NULL,L"游戏初始化失败",L"插件错误",MB_OK);
		return false;
	}
	on_mouse_button=(pfnOnMouseButton)::GetProcAddress(m_hGameLoader,"on_mouse_button");
	on_mouse_move=(pfnOnMouseMove)::GetProcAddress(m_hGameLoader,"on_mouse_move");
	on_key_down=(pfnOnKeyEvent)::GetProcAddress(m_hGameLoader,"on_key_down");
	on_key_up=(pfnOnKeyEvent)::GetProcAddress(m_hGameLoader,"on_key_up");
	return true;
}

