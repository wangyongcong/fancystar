#include "fscorepch.h"

// Windows library
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "wininet.lib")

// OpenGL library
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glu32.lib")
#pragma comment (lib, "glew32mx.lib")

// FreeImage library
#pragma comment (lib, "freeimage.lib")

// FreeType font library
#ifdef _DEBUG
	#pragma comment (lib, "freetype248_d.lib")
#else
	#pragma comment (lib, "freetype248.lib")
#endif
