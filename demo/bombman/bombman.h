#include "fscorepch.h"
#include "xgame.h"
#include "xcamera.h"

using wyc::xpointer;
using wyc::xobject;
using wyc::xobjptr;
using wyc::xobject_group;

class xgame_bombman : public wyc::xgame
{
	xpointer<xobject_group> m_spGroup;
	xpointer<wyc::xcamera> m_spCamera;
	xobjptr m_spMap;
	unsigned m_mapw, m_maph;
	bool m_lbdown, m_rbdown;
public:
	xgame_bombman();
	virtual bool init_game();
	virtual void quit_game();
	virtual void process_input(float elapsed);
	virtual void process_logic(float interval);
};


