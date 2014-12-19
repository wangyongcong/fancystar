#include "fscorepch.h"
#include "xgame.h"
#include "xcamera.h"

using wyc::xpointer;
using wyc::xobject;
using wyc::xobjptr;
using wyc::xobject_group;

class xgame_texmaker : public wyc::xgame
{
	xpointer<xobject_group> m_spGroup;
	wyc::xasync_queue m_msgque;
	bool m_lbdown, m_rbdown;
public:
	xgame_texmaker();
	virtual bool init_game();
	virtual void quit_game();
	virtual void process_input();
	virtual void process_logic(double accum_time, double frame_time);
	virtual bool on_window_message(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT &ret);
};


