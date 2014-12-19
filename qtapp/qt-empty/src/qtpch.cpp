#include "qtpch.h"

// OpenGL library
#pragma comment (lib, "OpenGL32.lib")
#pragma comment (lib, "GLU32.lib")
//#pragma comment (lib, "glew32mx.lib")

// Qt library
#if _DEBUG
	#pragma comment (lib, "qtmaind.lib")
	#pragma comment (lib, "Qt5Cored.lib")
	#pragma comment (lib, "Qt5Guid.lib")
	#pragma comment (lib, "Qt5OpenGLd.lib")
#else
	#pragma comment (lib, "qtmain.lib")
	#pragma comment (lib, "Qt5Core.lib")
	#pragma comment (lib, "Qt5Gui.lib")
	#pragma comment (lib, "Qt5OpenGL.lib")
#endif // _DEBUG