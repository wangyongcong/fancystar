#include "fscorepch.h"
#include "xobject2d.h"
#include "renderer.h"
#include "system2d.h"

namespace wyc 
{

BEGIN_EVENT_MAP(xobject2d,xobject)
END_EVENT_MAP

unsigned xobject2d::ms_renderPassID=0;
unsigned xobject2d::ms_pickerPassID=0;

xobject2d::xobject2d() 
{
	m_parent=0;
	m_flag=0;
	m_pos.set_zero();
	m_size.set_zero();
	m_rotate=0;
	memset(m_color,0xFF,sizeof(uint32_t)*4);
	m_kp.set_zero();
	m_pickCode=0;
	m_alphaFilter=1;
	m_pFrameCode=0;
}

void xobject2d::on_destroy() 
{
	remove_proxy();
	for(size_t i=0; i<m_children.size(); ++i) {
		m_children[i]->decref();
	}
	m_children.clear();
	if(m_pickCode) {
		xsystem2d::get_system2d()->free_pick_code(1,&m_pickCode);
		m_pickCode=0;
	}
	if(m_pFrameCode) {
		if(have_state(m_flag,OBJ2D_FRAME_ENABLE)) 
			xsystem2d::get_system2d()->free_pick_code(8,m_pFrameCode);
		delete [] m_pFrameCode;
		m_pFrameCode=0;
	}
}

void xobject2d::add_child(xobject2d *pobj)
{
	if(pobj->m_parent==this)
		return;
	if(0==pobj->get_context())
		pobj->assign_context(get_context());
	assert(pobj->get_context()==get_context());
	pobj->incref();
	if(pobj->m_parent) 
		pobj->m_parent->remove_child(pobj);
	pobj->m_parent=this;
	pobj->m_flag|=OBJ2D_CHG_PARENT;
	m_children.push_back(pobj);
}

void xobject2d::remove_child(xobject2d *pobj)
{
	child_list_t::iterator iter=std::find(m_children.begin(),m_children.end(),pobj);
	if(iter!=m_children.end()) {
		assert(*iter==pobj);
		assert(this==pobj->m_parent);
		pobj->m_parent=0;
		pobj->m_flag|=OBJ2D_CHG_PARENT;
		pobj->decref();
		*iter=0;
	}
}

void xobject2d::update_children(double accum_time, double frame_time)
{
	xobject2d *pobj;
	size_t i=0, size=m_children.size(), last;
	last=size;
	while(i<last) {
		pobj=m_children[i];
		if(!pobj || pobj->dead()) {
			if(pobj) {
				pobj->m_parent=0;
				pobj->decref();
			}
			--last;
			m_children[i]=m_children[last];
			continue;
		}
		pobj->update_children(accum_time,frame_time);
		pobj->update(accum_time,frame_time);
		pobj->sync_data();
		++i;
	}
	if(last!=size) { 
		m_children.erase(m_children.begin()+last,m_children.begin()+size);
	}
}

void xobject2d::create_proxy() 
{
	xrenderer *pRenderer=xrenderer::get_renderer();
	xobjevent *pev=xpackev::pack("sdde","xobject2d_render",id(),0,0);
	pRenderer->send_event(xrenderer::EV_CREATE_PROXY,pev);	
}

void xobject2d::remove_proxy()
{
	xrenderer *pRenderer=xrenderer::get_renderer();
	pRenderer->send_event(xrenderer::EV_REMOVE_PROXY,xpackev::pack("d",id()));	
}

void xobject2d::set_pick(bool enable, PICK_TYPE type, float filter)
{	
	m_flag&=~OBJ2D_PICK_MASK;
	if(enable) {
		m_flag|=type<<OBJ2D_PICK_SHIFT;
		if(!m_pickCode) {
			xsystem2d *psys=xsystem2d::get_system2d();
			psys->alloc_pick_code(1,&m_pickCode);
			psys->add_mouse_handler(m_pickCode,this);
		}
		m_alphaFilter=type==PICK_ALPHA?filter:1;

	}
	else if(m_pickCode) {
		xsystem2d::get_system2d()->free_pick_code(1,&m_pickCode);
		m_pickCode=0;
	}
	m_flag|=OBJ2D_CHG_PICK;
}

void xobject2d::get_world_pos(xvec2f_t &pos) const
{
	pos.x=m_pos.x, pos.y=m_pos.y;
	xobject2d *parent=m_parent;
	while(parent) {
		pos+=parent->get_pos();
		parent=parent->m_parent;
	}
}

void xobject2d::sync_data()
{
	xrenderer *pRenderer=xrenderer::get_renderer();
	if(have_state(m_flag,OBJ2D_CHG_PARENT)) {
		xpackev *notify=xpackev::pack("df",m_parent?m_parent->id():0,m_pos.z);
		pRenderer->send_event(xrenderer::EV_NOTIFY_PROXY,xpackev::pack("dde",id(),EV_SET_PARENT,notify));
	}
	if(have_state(m_flag,OBJ2D_CHG_VISIBLE)) {
		xpackev *notify=xpackev::pack("d",visible()?1:0);
		pRenderer->send_event(xrenderer::EV_NOTIFY_PROXY,xpackev::pack("dde",id(),EV_SET_VISIBLE,notify));
	}
	if(!visible())
		return;
	if(have_state(m_flag,OBJ2D_REBUILD_MESH)) {
		if(have_state(m_flag,OBJ2D_TRANSFORM)) {
			xpackev *notify=xpackev::pack("3ff2f",m_pos.elem,m_rotate,m_size.elem);
			pRenderer->send_event(xrenderer::EV_NOTIFY_PROXY,xpackev::pack("dde",id(),EV_TRANSFORM,notify));
		}
		if(have_state(m_flag,OBJ2D_CHG_COLOR)) {
			xpackev *notify=xpackev::pack("4d",m_color);
			pRenderer->send_event(xrenderer::EV_NOTIFY_PROXY,xpackev::pack("dde",id(),EV_SET_COLOR,notify));
		}
		if(have_state(m_flag,OBJ2D_CHG_IMAGE)) {
			xpackev *notify=xpackev::pack("sd2f",m_imgName.c_str(),m_flag&OBJ2D_BLIT_MASK,m_kp.elem);
			pRenderer->send_event(xrenderer::EV_NOTIFY_PROXY,xpackev::pack("dde",id(),EV_SET_IMAGE,notify));
		}
		else if(have_state(m_flag,OBJ2D_CHG_KEYPOINT)) {
			xpackev *notify=xpackev::pack("2f",m_kp.elem);
			pRenderer->send_event(xrenderer::EV_NOTIFY_PROXY,xpackev::pack("dde",id(),EV_SET_KEYPOINT,notify));
		}
	}
	if(have_state(m_flag,OBJ2D_CHG_PICK)) {
		xpackev *notify=xpackev::pack("ddf",pick_type(),m_pickCode,m_alphaFilter);
		pRenderer->send_event(xrenderer::EV_NOTIFY_PROXY,xpackev::pack("dde",id(),EV_SET_PICK,notify));
	}
	if(have_state(m_flag,OBJ2D_CHG_MASK)) {
		remove_state(m_flag,OBJ2D_CHG_MASK);
		pRenderer->send_event(xrenderer::EV_NOTIFY_PROXY,xpackev::pack("dde",id(),EV_PREPARE_DRAW,0));
	}
}

bool xobject2d::send_mouse_event(xobject2d *receiver, int evid, int x, int y, unsigned button)
{
	if(on_mouse_event(receiver,evid,x,y,button))
		return true;
	if(m_parent) {
		return m_parent->send_mouse_event(receiver,evid,x,y,button);
	}
	return xsystem2d::get_system2d()->on_mouse_event(receiver,evid,x,y,button);
}

bool xobject2d::on_mouse_event(xobject2d*, int, int, int, unsigned)
{
	return false;
}

void xobject2d::set_capture(bool b)
{
	if(!m_pickCode)
		return;
	xsystem2d *psys=xsystem2d::get_system2d();
	psys->set_capture(m_pickCode,b);
}

void xobject2d::enable_frame()
{
	if(m_flag&OBJ2D_FRAME_ENABLE)
		return;
	xsystem2d *psys=xsystem2d::get_system2d();
	if(!m_pFrameCode)
		m_pFrameCode=new uint32_t[8];
	psys->alloc_pick_code(8,m_pFrameCode);
	for(int i=0; i<8; ++i)
		psys->add_mouse_handler(m_pFrameCode[i],this);
	m_flag|=OBJ2D_FRAME_ENABLE;
	xpackev *notify=xpackev::pack("8d",m_pFrameCode);
	xrenderer::get_renderer()->send_event(xrenderer::EV_NOTIFY_PROXY,\
		xpackev::pack("dde",id(),EV_ENABLE_FRAME,notify));
}

void xobject2d::disable_frame()
{
	if(!have_state(m_flag,OBJ2D_FRAME_ENABLE))
		return;
	xsystem2d *psys=xsystem2d::get_system2d();
	psys->free_pick_code(8,m_pFrameCode);
	memset(m_pFrameCode,0,sizeof(uint32_t)*8);
	remove_state(m_flag,OBJ2D_FRAME_ENABLE);
	xrenderer::get_renderer()->send_event(xrenderer::EV_NOTIFY_PROXY,\
		xpackev::pack("dde",id(),EV_DISABLE_FRAME,0));
}

int xobject2d::get_frame(uint32_t code) const
{
	if(m_pFrameCode) {
		for(int i=0; i<8; ++i)
			if(m_pFrameCode[i]==code)
				return i;
	}
	return -1;
}

enum {
	FRAME_COLOR_NORMAL=0xFF00FF00,
	FRAME_COLOR_E=0xFFFF8080,
	FRAME_COLOR_W=0xFFFF8080,
	FRAME_COLOR_S=0xFFFF8080,
	FRAME_COLOR_N=0xFFFF8080,
	FRAME_COLOR_SE=0xFF4080FF,
	FRAME_COLOR_SW=0xFF4080FF,
	FRAME_COLOR_NE=0xFF4080FF,
	FRAME_COLOR_NW=0xFF4080FF,
};

void xobject2d::highlight_frame(int frame_id, bool b)
{
	static const int32_t ls_frameHighlightColor[8] = {
		FRAME_COLOR_SE, FRAME_COLOR_S, FRAME_COLOR_SW,
		FRAME_COLOR_E,                 FRAME_COLOR_W,
		FRAME_COLOR_NE, FRAME_COLOR_N, FRAME_COLOR_NW,
	};
	if(frame_id<0 || frame_id>=8)
		return;
	int32_t c=b?ls_frameHighlightColor[frame_id]:FRAME_COLOR_NORMAL;
	xpackev *pev=xpackev::pack("dd",frame_id,c);
	notify_renderer(EV_SET_FRAME_COLOR,pev);
}

}; // namespace wyc

