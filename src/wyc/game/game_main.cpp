#include "fscorepch.h"
#include <cstdio>
#include "wyc/util/util.h"
#include "wyc/game/game.h"

#ifdef _DEBUG
	#pragma comment (lib, "fsrender_d.lib")
#else
	#pragma comment (lib, "fsrender.lib")
#endif

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	setlocale(LC_ALL,"chs");

	extern void link_inner_module(void);
	link_inner_module();

	if(!wyc::xgame::init_game_process())
		return 1;
	wyc::xgame *pGame=wyc::xgame::create_game();
	wyc_log_init(pGame->code_name().c_str(),"logs",wyc::LOG_NORMAL,false);
	wyc_sys("FreeImage version %s",FreeImage_GetVersion());
	if(!pGame->create(hInstance,NULL,0,0,0,1024,768,false))
		return 2;
	int ret=pGame->run();
	wyc::xgame::exit_game_process();

	wyc_log_close();
	return ret;
}

#ifndef _LIB

void wyc::xrenderer::initialize_shaders()
{
}

wyc::xgame* wyc::xgame::create_game() 
{
	return new xgame();
}

#endif // _LIB
