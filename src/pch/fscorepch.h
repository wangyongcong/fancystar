#ifndef __FANCYSTAR_CORE_PCH
#define __FANCYSTAR_CORE_PCH

#include "winpch.h"

#include "stlpch.h"

#ifndef GLEW_MX
#define GLEW_MX // enable multiple render context
#endif 
#include "GL/glew.h"
#include "GL/wglew.h"

extern GLEWContext* glewGetContext();
extern WGLEWContext* wglewGetContext();

#define GL_COMPAT

#include "freeimage.h"

#endif // __FANCYSTAR_CORE_PCH


