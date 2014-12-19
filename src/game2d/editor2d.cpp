#include "fscorepch.h"
#include "editor2d.h"
#include "input_cfg.h"

namespace wyc
{

BEGIN_EVENT_MAP(xeditor2d,xlayer)
END_EVENT_MAP

xeditor2d::xeditor2d()
{
	m_frameObj=0;
	m_resizeMode=-1;
	m_dragMode=false;
	m_rotMode=false;
}

void xeditor2d::on_destroy()
{
	unselect();
	xlayer::on_destroy();
}

void xeditor2d::create_proxy() 
{
	xrenderer *pRenderer=xrenderer::get_renderer();
	xobjevent *pev=xpackev::pack("sdde","xeditor2d_render",id(),0,xpackev::pack("dd",\
		xobject2d::ms_renderPassID,xobject2d::ms_pickerPassID));
	pRenderer->send_event(xrenderer::EV_CREATE_PROXY,pev);
}

void xeditor2d::init_editor()
{
	xobject2d *pobj;
	//-------------------------------------
	// TODO: debug object, remove it
	pobj=wycnew xobject2d;
	add_child(pobj);
	pobj->create_proxy();
	pobj->set_size(256,256);
	pobj->set_keypoint(128,128);
	pobj->set_pos(400,300);
	pobj->set_image("fs_chess_board",wyc::BLIT_HV);
	pobj->set_pick(true,wyc::PICK_QUAD);
	pobj->set_edit_mode(true);
	pobj->rotate(30);

	pobj=wycnew xobject2d;
	add_child(pobj);
	pobj->create_proxy();
	pobj->set_size(64,64);
	pobj->set_keypoint(32,32);
	pobj->set_pos(200,150);
	pobj->set_color(0xFF00FFFF,0xFF0000FF,0xFF00FFFF,0xFF0000FF);
	pobj->set_image("fs_blank",wyc::BLIT_HV);
	pobj->set_pick(true,wyc::PICK_QUAD);
	pobj->set_edit_mode(true);
	//-------------------------------------
}

void xeditor2d::update(double accum_time, double frame_time)
{
	xlayer::update(accum_time,frame_time);
}

bool xeditor2d::is_selected(xobject2d *pobj) const
{
	unsigned cnt=m_selectedObjs.size();
	for(unsigned i=0; i<cnt; ++i)
	{
		if(m_selectedObjs[i]==pobj)
			return true;
	}
	return false;
}

void xeditor2d::select(xobject2d *pobj, bool multiple)
{
	if(is_selected(pobj))
		return;
	if(!multiple) {
		unselect();
	}
	pobj->incref();
	pobj->enable_frame();
	m_selectedObjs.push_back(pobj);
}

void xeditor2d::unselect()
{
	xobject2d *pobj;
	unsigned cnt=m_selectedObjs.size();
	for(unsigned i=0; i<cnt; ++i)
	{
		pobj=m_selectedObjs[i];
		pobj->disable_frame();
		pobj->hide_frame();
		pobj->decref();
	}
	m_selectedObjs.clear();
}

void xeditor2d::begin_drag(xobject2d *pobj, uint32_t code, int x, int y)
{
	if(code!=pobj->pick_code()) 
		m_resizeMode=pobj->get_frame(code);
	else
		m_resizeMode=-1;
	m_dragMode=true;
	m_pickx=x;
	m_picky=y;
}

void xeditor2d::end_drag()
{
	m_dragMode=false;
}

void xeditor2d::on_mouse_move(int x, int y)
{
	int offx=x-m_pickx, offy=y-m_picky;
	m_pickx=x, m_picky=y;
	if(m_dragMode) {
		size_t cnt=m_selectedObjs.size();
		xobject2d *pobj;
		if(m_resizeMode==-1) {
			// move objects
			for(size_t i=0; i<cnt; ++i) {
				pobj=m_selectedObjs[i];
				const xvec3f_t &pos=pobj->get_pos();
				pobj->set_pos(pos.x+offx,pos.y-offy);
			}
		}
		else {
			// resize objects
			int offw=0, offh=0;
			switch(m_resizeMode)
			{
			case 0: // SE
				offw=-offx, offh=offy;
				offy=-offy;
				break;
			case 1: // S
				offw=0, offh=offy;
				offx=0, offy=-offy;
				break;
			case 2: // SW
				offw=offx, offh=offy;
				offx=0, offy=-offy;
				break;
			case 3: // E
				offw=-offx;
				offy=0;
				break;
			case 4: // W
				offw=offx;
				offx=0, offy=0;
				break;
			case 5: // NE
				offw=-offx, offh=-offy;
				offy=0;
				break;
			case 6: // N
				offh=-offy;
				offx=0, offy=0;
				break;
			case 7: // NW
				offw=offx, offh=-offy;
				offx=0, offy=0;
				break;
			};
			for(size_t i=0; i<cnt; ++i) {
				pobj=m_selectedObjs[i];
				const xvec3f_t& pos=pobj->get_pos();
				const xvec2f_t& size=pobj->get_size();
				pobj->set_pos(pos.x+offx, pos.y+offy);
				pobj->set_size(std::max(size.x+offw,1.0f),
					std::max(size.y+offh,1.0f));
			}
		}
	}
}

void xeditor2d::begin_rotate(int x, int y)
{
	m_rotMode=true;
	x,y;
}

void xeditor2d::end_rotate()
{
	m_rotMode=false;
}

}; // namespace wyc

