#include "fscorepch.h"

#ifndef _LIB
	#ifdef _DEBUG
		#pragma comment (lib, "fsutil_d.lib")
		#pragma comment (lib, "fslog_d.lib")
		#pragma comment (lib, "fsthread_d.lib")
		#pragma comment (lib, "fsmemory_d.lib")
		#pragma comment (lib, "fsmath_d.lib")
		#pragma comment (lib, "fsobject_d.lib")
		#pragma comment (lib, "fsgame_d.lib")
		#pragma comment (lib, "fsrender_d.lib")
		#pragma comment (lib, "fsgui_d.lib")
	#else
		#pragma comment (lib, "fsutil.lib")
		#pragma comment (lib, "fslog.lib")
		#pragma comment (lib, "fsthread.lib")
		#pragma comment (lib, "fsmemory.lib")
		#pragma comment (lib, "fsmath.lib")
		#pragma comment (lib, "fsobject.lib")
		#pragma comment (lib, "fsgame.lib")
		#pragma comment (lib, "fsrender.lib")
		#pragma comment (lib, "fsgui.lib")
	#endif // _DEBUG
#endif // ! _LIB
