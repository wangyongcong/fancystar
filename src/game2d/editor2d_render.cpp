#include "fscorepch.h"
#include "xlayer_render.h"

EXPORT_STATIC_MODULE(xeditor2d_render);

namespace wyc
{

class xeditor2d_render : public xlayer_render
{
	USE_EVENT_MAP;
public:
	xeditor2d_render();
	virtual void on_destroy();
	virtual void draw(xrenderer *pRenderer);
	virtual void draw_picker(xrenderer *pRenderer);
protected:
	void ev_prepare_draw(xobjevent *pev);
};

BEGIN_EVENT_MAP(xeditor2d_render,xlayer_render)
	REG_EVENT(ev_prepare_draw)
END_EVENT_MAP

xeditor2d_render::xeditor2d_render() 
{
}

void xeditor2d_render::on_destroy() 
{
	xlayer_render::on_destroy();
}

void xeditor2d_render::draw(xrenderer *pRenderer)
{
	xlayer_render::draw(pRenderer);
}

void xeditor2d_render::draw_picker(xrenderer *pRenderer)
{
	xlayer_render::draw_picker(pRenderer);
}

void xeditor2d_render::ev_prepare_draw(xobjevent *pev)
{
	xlayer_render::ev_prepare_draw(pev);
}

};

