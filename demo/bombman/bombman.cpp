#include "fscorepch.h"
#include "bombman.h"
#include "xobject2d.h"
#include "game_config.h"

using wyc::xvec3f_t;
using wyc::xpointer;
using wyc::xgame;
using wyc::xrenderer;
using wyc::xobject;
using wyc::xobject_group;
using wyc::xcamera;
using wyc::xobjevent;
using wyc::xpackev;

xgame_bombman::xgame_bombman() 
{
	m_lbdown=false;
	m_rbdown=false;
}

bool xgame_bombman::init_game() 
{
	xcamera *pcam=wycnew xcamera;
	pcam->assign_context();
	pcam->set_orthograph(0,0,-1,float(client_width()),float(client_height()),1);
/*
	wyc::xspherical<float> campos;
	campos.r=3;
	campos.longitude=DEG_TO_RAD(-90);
	campos.latitude=DEG_TO_RAD(90);
	pcam->assign_context(&m_pObjectContext[0]);
	pcam->set_perspective(45,float(client_width())/client_height(),1,1000);
	pcam->look_at(xvec3f_t(0,0,campos.r),xvec3f_t(0,0,0),xvec3f_t(0,1,0));
*/
	m_spCamera=pcam;

	m_spGroup=wycnew xobject_group;
	m_spGroup->assign_context();

	wyc::xobject *pobj;
/*	pobj=wycnew wyc::xobject2d;
	pobj->assign_context(&m_pObjectContext[0]);
	m_spGroup->send_event(xobject_group::EV_JOIN, xpackev::pack("od",pobj,0));
	pobj->send_event(wyc::xobject2d::EV_SET_IMAGE, xpackev::pack("ssd","avatar.jpg","",0));
*/
	pobj=wyc::xobject::create_object("xtilemap");
	assert(pobj);
	m_spGroup->send_event(xobject_group::EV_JOIN, xpackev::pack("od",pobj,0));
	m_mapw=20, m_maph=15;
	pobj->send_event("set_camera",xpackev::pack("o",pcam));
	pobj->send_event("create_map", xpackev::pack("dddd",m_mapw,m_maph,40,40));
	m_spMap=pobj;
	m_mapw*=GRID_WIDTH;
	m_maph*=GRID_HEIGHT;
	return true;
}

void xgame_bombman::quit_game() 
{
	m_spMap=0;
	m_spCamera=0;
	m_spGroup=0;
}

void xgame_bombman::process_input(float elapsed)
{
	input().update_input();
	const wyc::xinput::xinputbuffer &ibuff=input().get_buffer();
	// handle mouse message
	wyc::xinput::xmouseque::const_iterator iter, end;
	iter=ibuff.mouseque.begin();
	end=ibuff.mouseque.end();
	while(iter!=end) {
		switch(iter->msg) {
		case MOUSEST_LBDOWN:
			m_lbdown=true;
			if(!m_rbdown) {
				const xvec3f_t &campos=m_spCamera->get_pos();
				m_spMap->send_event("edit_put_tile",xpackev::pack("ff",campos.x+ibuff.x,\
					campos.y+client_height()-1-ibuff.y));
			}
			break;
		case MOUSEST_LBUP:
			m_lbdown=false;
			break;
		case MOUSEST_RBDOWN:
			m_rbdown=true;
			break;
		case MOUSEST_RBUP:
			m_rbdown=false;
			break;
		}
		++iter;
	}
	if(ibuff.offx!=0 || ibuff.offy!=0) {
		if(m_rbdown) {
			xvec3f_t pos=m_spCamera->get_pos();
			pos.x+=ibuff.offx*SCROLL_SPEED;
			pos.y-=ibuff.offy*SCROLL_SPEED;
			unsigned xmax=m_mapw+GRID_WIDTH-client_width(),
				ymax=m_maph+GRID_HEIGHT-client_height();
			if(pos.x<-GRID_WIDTH)
				pos.x=-GRID_WIDTH;
			else if(pos.x>xmax)
				pos.x=xmax;
			if(pos.y<-GRID_HEIGHT)
				pos.y=-GRID_HEIGHT;
			else if(pos.y>ymax)
				pos.y=ymax;
			m_spCamera->set_pos(pos.x,pos.y,pos.z);
		}
		else if(m_lbdown) {
			const xvec3f_t &campos=m_spCamera->get_pos();
			m_spMap->send_event("edit_paint_tiles",xpackev::pack("ff",campos.x+ibuff.x,\
				campos.y+client_height()-1-ibuff.y));
		}
	}
	// handle keyboard message
	wyc::xinput::xkeyque::const_iterator kiter, kend;
	kiter=ibuff.keyque.begin();
	kend=ibuff.keyque.end();
	while(kiter!=kend) {
		if(*kiter&KEYST_KEYUP) {
			unsigned code=*kiter&KEY_CODE_MASK;
			if(code>='1' && code<='9') {
				unsigned tileID=code-'1';
				m_spMap->send_event("edit_set_tile",xpackev::pack("d",tileID));
			}
			else {
				switch(code) {
				case 'G':
					m_spMap->send_event("switch_grid_show",new xobjevent);
					break;
				}
			}
		}
		++kiter;
	}
}

void xgame_bombman::process_logic(float interval) 
{
	xgame::process_logic(interval);
	m_spCamera->update(interval);
	m_spGroup->update(interval);
}

