#ifndef __HEADER_WYC_COM_RENDER
#define __HEADER_WYC_COM_RENDER

#include "wyc/render/renderer.h"
#include "wyc/world/component.h"

namespace wyc
{

class xcom_renderer : public xcomponent
{
	USE_RTTI;
public:
	xcom_renderer();
	virtual void before_render(xrenderer *pRenderer);
	virtual void render(xrenderer *pRenderer);
	virtual void after_render(xrenderer *pRenderer);
};

}; // namespace wyc

#endif // __HEADER_WYC_COM_RENDER
