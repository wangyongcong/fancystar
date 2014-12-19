#ifndef _LIB

#include <windows.h>
#include <string>
#include <iostream>
#include "xutil.h"
#include "xconsole.h"

#ifdef _DEBUG
	#pragma comment (lib, "fshash_d.lib")
	#pragma comment (lib, "fsstr_d.lib")
#else
	#pragma comment (lib, "fshash.lib")
	#pragma comment (lib, "fsstr.lib")
#endif

using namespace wyc;

int __stdcall ::WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	if(!xconsole::create_console()) {
		wyc_error("can not create console!");
		return 1;
	}
	xconsole::conflag(1);
	print_libinfo();
	wyc_print("please enter command:");
	xconsole::singleton().process_input();
	xconsole::destroy_console();
	return 0;
}

#endif

