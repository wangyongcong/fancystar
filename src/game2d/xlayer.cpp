#include "fscorepch.h"
#include "xlayer.h"
#include "renderer.h"

namespace wyc
{

#define PREPARE_LAYER_DRAW	((void*)1)
#define PREPARE_PICKER_DRAW	((void*)2)
#define DRAW_LAYER	((void*)3)
#define DRAW_PICKER	((void*)4)
#define PREPARE_DRAW ((void*)5)
#define UPDATE_TILEBUFFER ((void*)6)

BEGIN_EVENT_MAP(xlayer,xobject2d)
END_EVENT_MAP

xlayer::xlayer()
{
}

void xlayer::on_destroy()
{
	xobject2d::on_destroy();
}

void xlayer::create_proxy() 
{
	xrenderer *pRenderer=xrenderer::get_renderer();
	xobjevent *pev=xpackev::pack("sdde","xlayer_render",id(),0,xpackev::pack("dd",\
		xobject2d::ms_renderPassID,xobject2d::ms_pickerPassID));
	pRenderer->send_event(xrenderer::EV_CREATE_PROXY,pev);
}

void xlayer::update(double accum_time, double frame_time)
{
	update_children(accum_time,frame_time);
	sync_data();
}

}; // namespace wyc

