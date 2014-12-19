#include "fscorepch.h"
#include <shellapi.h>
#include "strutil.h"
#include "texmaker.h"
#include "system2d.h"

void dummy_function()
{
	IMPORT_STATIC_MODULE(xobject2d_render);
	IMPORT_STATIC_MODULE(xlayer_render);
	IMPORT_STATIC_MODULE(xeditor2d_render);
}

using wyc::xvec3f_t;
using wyc::xpointer;
using wyc::xgame;
using wyc::xrenderer;
using wyc::xobject;
using wyc::xobject_group;
using wyc::xcamera;
using wyc::xobjevent;
using wyc::xpackev;

xgame_texmaker::xgame_texmaker() 
{
	m_lbdown=false;
	m_rbdown=false;
}

bool xgame_texmaker::on_window_message(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT &ret)
{
	if(msg==WM_DROPFILES) {
		unsigned fileCount=DragQueryFile((HDROP)wparam,0xFFFFFFFF,0,0);
		wchar_t filePath[MAX_PATH];
		unsigned len;
		std::string file;
		for(unsigned i=0; i<fileCount; ++i) {
			len=DragQueryFile((HDROP)wparam,i,filePath,MAX_PATH);
			wyc::wstr2str(file,filePath,len);
			m_msgque.push(xpackev::pack("s",file.c_str()));
		}
		DragFinish((HDROP)wparam);
		ret=0;
		return true;
	}
	return false;
}

bool xgame_texmaker::init_game() 
{
	DragAcceptFiles(main_window(),TRUE);

	
	xcamera *pcam=wycnew xcamera;
	pcam->assign_context();
	pcam->set_orthograph(0,0,-1,client_width(),client_height(),1);

	wyc::xsystem2d::init_system2d(client_width(),client_height(),pcam);

	m_spGroup=wycnew xobject_group;
	m_spGroup->assign_context();

	return true;
}

void xgame_texmaker::quit_game() 
{
	m_spGroup=0;
	wyc::xsystem2d::clear_system2d();
}

void xgame_texmaker::process_input()
{
	input().update_input();
	const wyc::xinput::xinputbuffer &ibuff=input().get_buffer();
	wyc::xsystem2d *pSystem=wyc::xsystem2d::get_system2d();
	wyc::xinput::xmouseque::const_iterator iter, end;
	iter=ibuff.mouseque.begin();
	end=ibuff.mouseque.end();
	while(iter!=end) {
		pSystem->dispatch_mouse_event(iter->msg,iter->x,iter->y,iter->button);
		++iter;
	}
	if(ibuff.offx!=0 || ibuff.offy!=0)
		pSystem->dispatch_mouse_event(wyc::EV_MOUSE_MOVE,ibuff.x,ibuff.y,0);
}

void xgame_texmaker::process_logic(double accum_time, double frame_time) 
{
	wyc::xsystem2d *psys=wyc::xsystem2d::get_system2d();
	wyc::xasync_queue::ENTRY *pentry=m_msgque.flush(), *pdel;
	wyc::xpackev *pev;
	const char *pimg;
	while(pentry) {
		pev=(wyc::xpackev*)pentry->m_pdata;
		if(pev->unpack("s",&pimg)) {
			psys->debug_add_object(pimg);			
		}
		pev->delthis();
		pdel=pentry;
		pentry=(wyc::xasync_queue::ENTRY*)pentry->next();
		m_msgque.free(pdel);
	}
	m_spGroup->update(accum_time,frame_time);
	psys->update(accum_time,frame_time);
}

