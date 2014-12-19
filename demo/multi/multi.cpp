#include "winpch.h"
#include <cassert>
#include <cstdio>
#include "basedef.h"
#include "xlog.h"

#ifdef _DEBUG
	#pragma comment (lib, "fsconsole.lib")
	#pragma comment (lib, "helloworld_d.lib")
#else
	#pragma comment (lib, "fsconsole.lib")
	#pragma comment (lib, "helloworld.lib")
#endif

// 激活的game instance数量
#define ENABLED_GAME_INSTANCE 2

using wyc::LOG_SYS;
using wyc::LOG_WARN;
using wyc::LOG_ERROR;

enum MOUSE_STATE 
{
	LEFT_BUTTON=0,
	RIGHT_BUTTON,
	MIDDLE_BUTTON,
	BUTTON_DOWN=0x10,
};

LRESULT WINAPI MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT WINAPI GameWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

typedef void* (*ProcLoadGame) (HINSTANCE hInstance, HWND hWnd, const wchar_t *pGameName, int x, int y, int width, int height);
typedef void  (*ProcClearGame) (void *pctx);
typedef void  (*ProcOnMouseButton) (void *pctx, int state, int x, int y);
typedef void  (*ProcOnMouseMove) (void *pctx, int x, int y);
typedef void  (*ProcOnMouseWheel) (void *pctx, int delta);

struct CGameInstance 
{
	HWND m_hWnd;
	void *m_pContext;
	ProcLoadGame load_game;
	ProcClearGame clear_game;
	ProcOnMouseButton on_mouse_button;
	ProcOnMouseMove on_mouse_move;
	ProcOnMouseWheel on_mouse_wheel;
	CGameInstance() {
		m_hWnd=NULL;
		m_pContext=0;
		load_game=0;
		clear_game=0;
		on_mouse_button=0;
		on_mouse_move=0;
		on_mouse_wheel=0;
	}
};

HWND s_hMainWnd=NULL;
CGameInstance s_gameInstance[4];

bool LoadGame(const wchar_t *pGameName, HINSTANCE hInstance, int w, int h, CGameInstance &gameInstance)
{
	wchar_t dllname[255];
	swprintf_s(dllname,255,L"%s_d.dll",pGameName);
	HMODULE hmod=LoadLibrary(dllname);
	if(hmod==NULL) {
		DWORD err=GetLastError();
		return false;
	}
	gameInstance.load_game=(ProcLoadGame)::GetProcAddress(hmod,"load_game");
	gameInstance.clear_game=(ProcClearGame)::GetProcAddress(hmod,"clear_game");
	gameInstance.on_mouse_button=(ProcOnMouseButton)::GetProcAddress(hmod,"on_mouse_button");
	gameInstance.on_mouse_move=(ProcOnMouseMove)::GetProcAddress(hmod,"on_mouse_move");
	gameInstance.on_mouse_wheel=(ProcOnMouseWheel)::GetProcAddress(hmod,"on_mouse_wheel");
	gameInstance.m_pContext=gameInstance.load_game(hInstance,gameInstance.m_hWnd,pGameName,0,0,w,h);
	return true;
}

int WINAPI ::WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR , int )
{
	// create main window
	const wchar_t *className=L"FSMultiMainWindow";
	WNDCLASSEX wndcls;
	if(!GetClassInfoEx(hInstance,className,&wndcls)) {
		wndcls.cbSize=sizeof(WNDCLASSEX);
		wndcls.style=CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
		wndcls.hInstance=hInstance;
		wndcls.lpfnWndProc=&MainWindowProc;
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
	DWORD style=WS_OVERLAPPEDWINDOW & (~WS_THICKFRAME);
	DWORD stylex=0;
	int x, y, width=815, height=615;
	RECT rectClient;
	rectClient.left=0, rectClient.top=0, rectClient.right=width, rectClient.bottom=height;
	AdjustWindowRectEx(&rectClient,style,false,stylex);

	RECT rectDesk;
	GetWindowRect(GetDesktopWindow(),&rectDesk);
	x=(rectDesk.right-rectDesk.left-rectClient.right+rectClient.left)>>1;
	y=(rectDesk.bottom-rectDesk.top-rectClient.bottom+rectClient.top)>>1;
	s_hMainWnd=CreateWindowEx(stylex, className, L"MainW", style, 
		x, y, rectClient.right-rectClient.left, rectClient.bottom-rectClient.top, NULL, NULL, hInstance, NULL);
	assert(s_hMainWnd!=NULL);
	ShowWindow(s_hMainWnd,SW_NORMAL);
	
	// create sub windows
	const wchar_t *className2=L"FSMultiGameWindow";
	if(!GetClassInfoEx(hInstance,className2,&wndcls)) {
		wndcls.cbSize=sizeof(WNDCLASSEX);
		wndcls.style=CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
		wndcls.hInstance=hInstance;
		wndcls.lpfnWndProc=&GameWindowProc;
		wndcls.cbClsExtra=0;
		wndcls.cbWndExtra=0;
		wndcls.hCursor=LoadCursor(NULL,IDC_ARROW);
		wndcls.hIcon=NULL;
		wndcls.hIconSm=NULL;
		wndcls.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
		wndcls.lpszMenuName=NULL;
		wndcls.lpszClassName=className2;
		if(!RegisterClassEx(&wndcls)) 
			return NULL;
	}
	style=WS_CHILDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	stylex=0;
	const wchar_t *subWndName[4]={
		L"Game0",
		L"Game1",
		L"Game2",
		L"Game3",
	};
	x=5, y=5;
	for(int i=0; i<4; ++i) {
		s_gameInstance[i].m_hWnd=CreateWindowEx(stylex, className2, subWndName[i], style, 
			x, y, 400, 300, s_hMainWnd, NULL, hInstance, NULL);
		assert(s_gameInstance[i].m_hWnd!=NULL);
		ShowWindow(s_gameInstance[i].m_hWnd,SW_NORMAL);
		if(i&1)  {
			x=5;
			y+=305;
		}
		else x+=405;
	}
	for(int i=0; i<ENABLED_GAME_INSTANCE; ++i)
		LoadGame(L"helloworld",hInstance,400,300,s_gameInstance[i]);

	wyc_log(LOG_SYS,"Fancystar Multi Porgram");
	HACCEL hAccel=NULL;
	MSG msg;
	while(GetMessage(&msg,NULL,0,0)) {
		if(!TranslateAccelerator(s_hMainWnd,hAccel,&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	for(int i=0; i<4; ++i) {
		if(s_gameInstance[i].m_pContext) 
			s_gameInstance[i].clear_game(s_gameInstance[i].m_pContext);
	}
	return 0;
}

#define GETX_LPARAM(lparam) short(lparam&0xFFFF)
#define GETY_LPARAM(lparam) short((lparam>>16)&0xFFFF)

LRESULT WINAPI MainWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	wchar_t text[256];
	switch(msg)
	{
	case WM_KEYUP:
		if(wparam==VK_ESCAPE) {
			PostMessage(s_hMainWnd,WM_CLOSE,0,0);
			return 0;
		}
		break;
	case WM_LBUTTONDOWN:
		::GetWindowText(hwnd,text,255);
		wyc_log(0,"%S: (%d,%d)",text,GETX_LPARAM(lparam),GETY_LPARAM(lparam));
		return 0;
	case WM_MOUSEWHEEL:
		return 0;
	case WM_CLOSE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT WINAPI GameWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	CGameInstance *pGame=0;
	for(int i=0; i<4; ++i) {
		if(s_gameInstance[i].m_hWnd==hwnd) {
			pGame=&s_gameInstance[i];
			break;
		}
	}
	if(pGame==0 || pGame->m_pContext==0)
		return DefWindowProc(hwnd, msg, wparam, lparam);

	switch(msg)
	{
	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		pGame->on_mouse_button(pGame->m_pContext,LEFT_BUTTON|BUTTON_DOWN,GETX_LPARAM(lparam),GETY_LPARAM(lparam));
		return 0;
	case WM_LBUTTONUP:
		ReleaseCapture();
		pGame->on_mouse_button(pGame->m_pContext,LEFT_BUTTON,GETX_LPARAM(lparam),GETY_LPARAM(lparam));
		return 0;
	case WM_RBUTTONDOWN:
		SetCapture(hwnd);
		pGame->on_mouse_button(pGame->m_pContext,RIGHT_BUTTON|BUTTON_DOWN,GETX_LPARAM(lparam),GETY_LPARAM(lparam));
		return 0;
	case WM_RBUTTONUP:
		ReleaseCapture();
		pGame->on_mouse_button(pGame->m_pContext,RIGHT_BUTTON,GETX_LPARAM(lparam),GETY_LPARAM(lparam));
		return 0;
	case WM_MOUSEMOVE:
		pGame->on_mouse_move(pGame->m_pContext,GETX_LPARAM(lparam),GETY_LPARAM(lparam));
		return 0;
	case WM_MOUSEWHEEL:
		pGame->on_mouse_wheel(pGame->m_pContext,GET_WHEEL_DELTA_WPARAM(wparam));
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
