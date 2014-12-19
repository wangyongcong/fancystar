#include "fscorepch.h"
#include "system2d.h"
#include "input_cfg.h"

#pragma warning (disable: 4245) // 有符号/无符号不匹配

namespace wyc
{

BEGIN_EVENT_MAP(xsystem2d,xobject)
END_EVENT_MAP

xthread_local xsystem2d::ms_tlsSystem2D;

xsystem2d *xsystem2d::ms_pSystem2d=0;

void xsystem2d::init_system2d(unsigned viewport_w, unsigned viewport_h, xcamera *pcam)
{
	ms_tlsSystem2D.alloc();

	xobject2d::ms_renderPassID=wyc::strhash("Layer Renderer"); 
	xobject2d::ms_pickerPassID=wyc::strhash("Layer Picker");

	xsystem2d *psys=wycnew xsystem2d;
	psys->assign_context();
	psys->incref();
	ms_tlsSystem2D.set_data(psys);
	psys->initialize(viewport_w,viewport_h,pcam);
}

void xsystem2d::clear_system2d()
{
	xsystem2d *psys=(xsystem2d*)ms_tlsSystem2D.get_data();
	psys->delthis();
	psys->decref();
	ms_tlsSystem2D.free();
}

xsystem2d* xsystem2d::get_system2d()
{
	return (xsystem2d*)ms_tlsSystem2D.get_data();
}

xsystem2d::xsystem2d()
{
	m_pickbuff=0;
	m_buffw=0;
	m_buffh=0;
	m_code=0;
	m_prevCode=0;
	m_capture=0;
	m_editMode=true;
}

void xsystem2d::on_destroy()
{
	// TODO: 不能删除
//	delete [] m_pickbuff;
	m_pickbuff=0;
	m_spCamera=0;
	m_spEditor->delthis();
	m_spEditor=0;
}

void xsystem2d::initialize(unsigned viewport_w, unsigned viewport_h, xcamera *pcam)
{
	size_t sz=viewport_w*viewport_h;
#ifdef _DEBUG
	m_pickbuff=new uint32_t[sz+1];
	// memory fence
	m_pickbuff[sz]=0;
#else
	m_pickbuff=new uint32_t[sz];
#endif // _DEBUG
	memset(m_pickbuff,0,sizeof(uint32_t)*sz);
	m_buffw=viewport_w, m_buffh=viewport_h;
	m_spCamera=pcam;

	xrenderer *pRenderer=xrenderer::get_renderer();
	// create pick pass
	xpackev *pevCreate=xpackev::pack("ddp",viewport_w,viewport_h,m_pickbuff);
	pRenderer->send_event(xrenderer::EV_CREATE_PROXY,xpackev::pack("sdde","xlayer_pickpass",\
		xobject2d::ms_pickerPassID,RENDER_PRIORITY(1),pevCreate));
	// create render pass
	pevCreate=xpackev::pack("dd",viewport_w,viewport_h);
	pRenderer->send_event(xrenderer::EV_CREATE_PROXY,xpackev::pack("sdde","xlayer_pass",\
		xobject2d::ms_renderPassID,RENDER_PRIORITY(2),pevCreate));

	m_spEditor=wycnew xeditor2d;
	m_spEditor->assign_context();
	m_spEditor->create_proxy();
	m_spEditor->set_size(float(viewport_w),float(viewport_h));
	m_spEditor->init_editor();
}

xobject2d* xsystem2d::get_object_at(int x, int y, uint32_t &code)
{
	int idx=x+(m_buffh-1-y)*m_buffw;
	if(idx>=0 && unsigned(idx)<m_buffw*m_buffh) {
		code=m_pickbuff[idx]&0xFFFFFF;
		if(code==0) 
			return 0;
		xobject2d *pobj;
		if(m_pickmap.get(code,(uintptr_t&)pobj))
			return pobj;
	}
	code=0;
	return 0;
}

bool xsystem2d::dispatch_mouse_event(int evid, int x, int y, unsigned button)
{
	uint32_t code;
	xobject2d *pobj=get_object_at(x,y,code);
	if(m_editMode) 
	{
		if(EV_MOUSE_MOVE==evid)
			edit_on_mouse_move(pobj,code,x,y);
		else
			edit_on_mouse_button(pobj,code,evid,x,y,button);
		return true;
	}
	if(evid==EV_MOUSE_MOVE) 
		return on_mouse_move(pobj,x,y,button);
	if(m_capture) {
		if(!m_pickmap.get(m_capture,(uintptr_t&)pobj)) {
			m_capture=0;
		}
	}			
	if(pobj) 
		return pobj->send_mouse_event(pobj,evid,x,y,button);
	return on_mouse_event(0,evid,x,y,button);	
}

void xsystem2d::edit_on_mouse_move(xobject2d *receiver, uint32_t code, int x, int y)
{
	if(m_spEditor->is_drag_mode()) {
		m_spEditor->on_mouse_move(x,y);
		return;
	}
	if(code==m_prevCode)
		return;
	xobject2d *pold;
	int old_frame, cur_frame;
	bool is_same=false;
	if(m_prevCode && m_pickmap.get(m_prevCode,(uintptr_t&)pold)) 
	{
		if(pold==receiver) {
			if(receiver->frame_enabled()) {
				old_frame=receiver->get_frame(m_prevCode);
				cur_frame=receiver->get_frame(code);
				if(old_frame!=-1)
					receiver->highlight_frame(old_frame,false);
				if(cur_frame!=-1)
					receiver->highlight_frame(cur_frame,true);
			}
			is_same=true;
		}
		else {
			if(pold->edit_mode()) {
				if(pold->frame_enabled()) {
					old_frame=pold->get_frame(m_prevCode);
					if(old_frame!=-1)
						pold->highlight_frame(old_frame,false);
				}
				else
					pold->hide_frame();
			}
		}
	}
	if(!is_same && receiver && receiver->edit_mode()) {
		if(receiver->frame_enabled()) {
			cur_frame=receiver->get_frame(code);
			if(cur_frame!=-1)
				receiver->highlight_frame(cur_frame,true);
		}
		else
			receiver->show_frame();
	}
	m_prevCode=code;
}

bool xsystem2d::edit_on_mouse_button(xobject2d *receiver, uint32_t code, int evid, int x, int y, unsigned button)
{
	switch(evid)
	{
	case EV_LB_DOWN:
		if(receiver && receiver->edit_mode()) {
			m_spEditor->select(receiver,0!=(button&MK_CONTROL));
			m_spEditor->begin_drag(receiver,code,x,y);
		}
		else m_spEditor->unselect();
		break;
	case EV_LB_UP:
		m_spEditor->end_drag();
		break;
	case EV_RB_DOWN:
		break;
	case EV_RB_UP:
		break;
	}
	return true;
}

bool xsystem2d::on_mouse_event(xobject2d*, int , int , int , unsigned )
{
	return false;
}

bool xsystem2d::on_mouse_move(xobject2d *receiver, int x, int y, unsigned button)
{
	xobject2d *pobj;
	wyc::uint32_t code=receiver?receiver->pick_code():0;
	if(m_capture) {
		if(m_pickmap.get(m_capture,(uintptr_t&)pobj)) {
			if(code!=m_capture) {
				if(m_prevCode==m_capture)
					pobj->send_mouse_event(pobj,EV_MOUSE_OUT,x,y,button);
			}
			else if(m_prevCode!=m_capture)
				pobj->send_mouse_event(pobj,EV_MOUSE_IN,x,y,button);
			pobj->send_mouse_event(pobj,EV_MOUSE_MOVE,x,y,button);
			m_prevCode=code;
			return true;
		}
		m_capture=0;
	}
	if(code!=m_prevCode) {
		if(m_prevCode) {
			if(m_pickmap.get(m_prevCode,(uintptr_t&)pobj)) 
				pobj->send_mouse_event(pobj,EV_MOUSE_OUT,x,y,button);
		}
		if(receiver) 
			receiver->send_mouse_event(receiver,EV_MOUSE_IN,x,y,button);
		m_prevCode=code;
	}
	if(receiver)
		return receiver->send_mouse_event(receiver,EV_MOUSE_MOVE,x,y,button);
	return false;
}

#define MAX_PICK_CODE 0xFFFFFF

void xsystem2d::add_mouse_handler(uint32_t code, xobject2d *pobj)
{
#ifdef _DEBUG
	assert(code);
	if(m_pickmap.contain(code)) {
		uintptr_t pold;
		m_pickmap.get(code,pold);
		if(pold!=(uintptr_t)pobj) {
			wyc_error("xsystem2d::add_mouse_handler: pick code conflict [%x]",code);
		}
		else {
			wyc_warn("xsystem2d::add_mouse_handler: dummy");
		}
	}
#endif 
	m_pickmap.add(code,(uintptr_t)pobj);
}

void xsystem2d::alloc_pick_code(unsigned count, uint32_t *pc)
{
	if(m_codepool.size()) {
		std::vector<uint32_t>::iterator beg=m_codepool.begin(), iter;
		iter=beg+m_codepool.size()-1;
		while(count>0 && iter!=beg) {
			*pc++=*iter;
			--iter;
			--count;
		}
		m_codepool.erase(iter,m_codepool.end());
	}
	while(count>0 && m_code<MAX_PICK_CODE) {
		*pc++=++m_code;
		--count;
	}
	assert(0==count);
}

void xsystem2d::free_pick_code(unsigned count, uint32_t *pc)
{
	uint32_t code;
	uintptr_t pobj;
	for(unsigned i=0; i<count; ++i) {
		code=pc[i]&0xFFFFFF;
		assert(code!=0 && code<=m_code);
		m_pickmap.pop(code,pobj);
		m_codepool.push_back(code);
		if(m_capture==code)
			m_capture=0;
		if(m_prevCode==code)
			m_prevCode=0;
	}
}

void xsystem2d::set_capture(uint32_t pc, bool b)
{
	if(b) {
		// 只有鼠标所在对象才能捕获鼠标
		// 如果鼠标已经被其他人所捕获,也无法抢夺所有权
		if(m_capture || m_prevCode!=pc)
			return;
		m_capture=pc;
	}
	else if(m_capture==pc) {
		m_capture=0;
	}
}

void xsystem2d::update(double accum_time, double frame_time)
{
	m_spCamera->update(accum_time,frame_time);

	xrenderer *pRenderer=xrenderer::get_renderer();

	unsigned nid=wyc::strhash("ev_mvp_matrix");
	xpackev *notify=xpackev::pack("16f",m_spCamera->get_mvpmatrix().data());
	xpackev *pev=xpackev::pack("dde",xobject2d::ms_pickerPassID,nid,notify);
	pRenderer->send_event(xrenderer::EV_NOTIFY_PROXY,pev);

	notify=xpackev::pack("16f",m_spCamera->get_mvpmatrix().data());
	pev=xpackev::pack("dde",xobject2d::ms_renderPassID,nid,notify);
	pRenderer->send_event(xrenderer::EV_NOTIFY_PROXY,pev);

	m_spEditor->update(accum_time,frame_time);
}

void xsystem2d::open_editor()
{
	m_editMode=true;
	m_capture=0;
	if(m_prevCode) {
		xobject2d *pobj;
		if(m_pickmap.get(m_prevCode,(uintptr_t&)pobj)) 
			pobj->send_mouse_event(pobj,EV_MOUSE_OUT,0,0,0);
		m_prevCode=0;
	}
}

void xsystem2d::close_editor()
{
	m_editMode=false;
	m_capture=0;
	if(m_prevCode) {
		xobject2d *pobj;
		if(m_pickmap.get(m_prevCode,(uintptr_t&)pobj)) 
			pobj->send_mouse_event(pobj,EV_MOUSE_OUT,0,0,0);
		m_prevCode=0;
	}
}

void xsystem2d::debug_add_object(const char *pimg)
{
	xobject2d *pobj=wycnew xobject2d;
	m_spEditor->add_child(pobj);
	pobj->create_proxy();
	pobj->set_image(pimg);
	pobj->set_size(100,100);
	pobj->set_pos(0,0);
	pobj->set_pick(true,wyc::PICK_QUAD);
}

}; // namespace wyc
