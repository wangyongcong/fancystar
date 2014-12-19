#include "fscorepch.h"

//-------------------------------------------------------------------------------
// DLL program
//-------------------------------------------------------------------------------

#ifdef _DLL

#ifdef WYC_EXPORT_GAME
	#undef WYC_IMPORT_GAME
	#define WYCAPI_GAME __declspec(dllexport)
#elif WYC_IMPORT_GAME
	#define WYCAPI_GAME __declspec(dllimport)
#else
	#define WYCAPI_GAME
#endif

#include "memtracer.h"
#include "bombman.h"

using wyc::xgame;

enum MOUSE_STATE 
{
	LEFT_BUTTON=0,
	RIGHT_BUTTON,
	MIDDLE_BUTTON,
	BUTTON_DOWN=0x10,
};

struct GAME_CONTEXT
{
	xgame *m_pGame;
};

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, void* lpReserved)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH: {
			wyc::xgame::init_game_process();
			char strbuff[MAX_PATH];
			int cnt;
			std::wstring path;
			path=wyc::home_path();
			path+=L"\\fancystar\\res\\";
			cnt=::WideCharToMultiByte(CP_ACP,0,path.c_str(),path.size(),strbuff,MAX_PATH,0,FALSE);
			strbuff[cnt]=0;
			wyc::set_resource_path(strbuff);
			path=wyc::home_path();
			path+=L"\\fancystar\\shader\\";
			cnt=::WideCharToMultiByte(CP_ACP,0,path.c_str(),path.size(),strbuff,MAX_PATH,0,FALSE);
			strbuff[cnt]=0;
			wyc::set_shader_path(strbuff);
			printf("Bombman.dll is attached!\n");
		}
		break;

	case DLL_PROCESS_DETACH:
		printf("Bombman.dll is detached!\n");
		wyc::xgame::exit_game_process();
		break;
	}
	return true;
}

extern "C"
{

WYCAPI_GAME void* load_game(HINSTANCE hInstance, HWND hWnd, const wchar_t *pGameName, int x, int y, int width, int height)
{
	xgame *pGame=new xgame_bombman;
	if(!pGame->create(hInstance,hWnd,pGameName,x,y,width,height,false)) {
		delete pGame;
		return 0;
	}
	GAME_CONTEXT *pGameCtx=new GAME_CONTEXT;
	pGameCtx->m_pGame=pGame;
	pGameCtx->m_pGame->run();
	return pGameCtx;
}

WYCAPI_GAME void clear_game(void *pctx) 
{
	GAME_CONTEXT *pGameCtx=(GAME_CONTEXT*)pctx;
	pGameCtx->m_pGame->exit();
	delete pGameCtx->m_pGame;
	delete pGameCtx;
}

WYCAPI_GAME void on_mouse_button(void *pctx, int state, int x, int y)
{
	GAME_CONTEXT *pGameCtx=(GAME_CONTEXT*)pctx;
	int btn=state&0xF;
	switch(btn) {
	case LEFT_BUTTON:
		if(state&BUTTON_DOWN)
			pGameCtx->m_pGame->input().lbtn_down(x,y);
		else 
			pGameCtx->m_pGame->input().lbtn_up(x,y);
		break;
	case RIGHT_BUTTON:
		if(state&BUTTON_DOWN)
			pGameCtx->m_pGame->input().rbtn_down(x,y);
		else
			pGameCtx->m_pGame->input().rbtn_up(x,y);
		break;
	case MIDDLE_BUTTON:
		if(state&BUTTON_DOWN)
			pGameCtx->m_pGame->input().mbtn_down(x,y);
		else
			pGameCtx->m_pGame->input().mbtn_up(x,y);
		break;
	}
}

WYCAPI_GAME void on_mouse_move(void *pctx, int x, int y)
{
	GAME_CONTEXT *pGameCtx=(GAME_CONTEXT*)pctx;
	pGameCtx->m_pGame->input().mouse_move(x,y);
}

WYCAPI_GAME void on_mouse_wheel(void *pctx, int delta)
{
	GAME_CONTEXT *pGameCtx=(GAME_CONTEXT*)pctx;
	pGameCtx->m_pGame->input().mouse_wheel(delta);
}

WYCAPI_GAME void on_key_down(void *pctx, unsigned key_code) 
{
	GAME_CONTEXT *pGameCtx=(GAME_CONTEXT*)pctx;
	pGameCtx->m_pGame->input().keydown(wyc::uint8_t(key_code));
}

WYCAPI_GAME void on_key_up(void *pctx, unsigned key_code) 
{
	GAME_CONTEXT *pGameCtx=(GAME_CONTEXT*)pctx;
	pGameCtx->m_pGame->input().keyup(wyc::uint8_t(key_code));
}

} // extern "C"


#endif // _DLL
