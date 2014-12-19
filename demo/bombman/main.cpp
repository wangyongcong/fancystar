#include "fscorepch.h"

#ifdef _DEBUG
	#pragma comment (lib, "fsgame_d.lib")
	#pragma comment (lib, "fsglrenderer_d.lib")
#else
	#pragma comment (lib, "fsgame.lib")
	#pragma comment (lib, "fsglrenderer.lib")
#endif

#include "bombman.h"

wyc::xgame* wyc::xgame::create_game() 
{
	return new xgame_bombman;
}

