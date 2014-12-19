#include "fscorepch.h"

//-------------------------------------------------------------------------------
// DLL program
//-------------------------------------------------------------------------------

#ifdef WYC_DLL

#ifdef WYC_EXPORT_GAME
	#undef WYC_IMPORT_GAME
	#define WYCAPI_GAME __declspec(dllexport)
#elif WYC_IMPORT_GAME
	#define WYCAPI_GAME __declspec(dllimport)
#else
	#define WYCAPI_GAME
#endif

#include "wyc/mem/memtracer.h"

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
	case DLL_PROCESS_ATTACH:
		wyc::xgame::init_game_process();
		wyc::set_resource_path("F:\\dev\\fancystar\\res\\");
		wyc::set_shader_path("F:\\dev\\fancystar\\shader\\");
		printf("HelloWorld.dll is attached!\n");
		break;

	case DLL_PROCESS_DETACH:
		printf("HelloWorld.dll is detached!\n");
		wyc::xgame::exit_game_process();
		break;
	}
	return true;
}

extern "C"
{

WYCAPI_GAME void* load_game(HINSTANCE hInstance, HWND hWnd, const wchar_t *pGameName, int x, int y, int width, int height)
{
	xgame *pGame=new xgame_helloworld;
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
		if(state&BUTTON_DOWN) {
			pGameCtx->m_pGame->input().lbtn_down(x,y);
		}
		else {
			pGameCtx->m_pGame->input().lbtn_up(x,y);
		}
		break;
	case RIGHT_BUTTON:
		if(state&BUTTON_DOWN) {
			pGameCtx->m_pGame->input().rbtn_down(x,y);
		}
		else {
			pGameCtx->m_pGame->input().rbtn_up(x,y);
		}
		break;
	case MIDDLE_BUTTON:
		if(state&BUTTON_DOWN) {
			pGameCtx->m_pGame->input().mbtn_down(x,y);
		}
		else {
			pGameCtx->m_pGame->input().mbtn_up(x,y);
		}
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


} // extern "C"


#endif // WYC_DLL