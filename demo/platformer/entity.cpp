#include "fscorepch.h"

#include "entity.h"

namespace demo
{

REG_RTTI(xentity, wyc::xobject)

xentity::xentity()
{
	m_pid = 0;
	m_agent=0;
	m_outline_color.set(1,1,1);
	m_fill_color.set(1,1,1);
	m_fill=false;
	m_selected=false;
}

void xentity::on_create(unsigned pid, int col_type, int col_group)
{
	if(m_pid) return;
	assert(pid);
	m_pid = pid;
	assert(m_agent==0);
	m_agent = new wyc::xcollision_agent(col_type);
	m_agent->set_data(this);
	m_agent->set_mask(col_group);
}

void xentity::on_destroy()
{
	if(m_agent) {
		delete m_agent;
		m_agent = 0;
	}
}

void xentity::draw(wyc::xrenderer *rd)
{
	const wyc::xvec2f_t &lower = m_agent->get_lower(), &upper=m_agent->get_upper();
	if(m_fill)
	{
		glColor3f(m_fill_color.x, m_fill_color.y, m_fill_color.z);
		glBegin(GL_QUADS);
			glVertex2f(lower.x,lower.y);
			glVertex2f(upper.x,lower.y);
			glVertex2f(upper.x,upper.y);
			glVertex2f(lower.x,upper.y);
		glEnd();
	}
	if(m_selected)
		glColor3f(0.0f, 1.0f, 0.0f);
	else
		glColor3f(m_outline_color.x, m_outline_color.y, m_outline_color.z);	
	glBegin(GL_LINE_STRIP);
		glVertex2f(lower.x,lower.y);
		glVertex2f(upper.x,lower.y);
		glVertex2f(upper.x,upper.y);
		glVertex2f(lower.x,upper.y);
		glVertex2f(lower.x,lower.y);
	glEnd();
}

//------------------------------------------------------

REG_RTTI(xobstacle, xentity);

void xobstacle::draw(wyc::xrenderer *rd)
{
	if(IS_SLOPE(m_agent->agent_type())) 
		_draw_slope(rd);
	else
		xentity::draw(rd);
}

void xobstacle::_draw_slope (wyc::xrenderer *rd) 
{
	const wyc::xvec2f_t &lower = m_agent->get_lower(), &upper=m_agent->get_upper();
	if(m_selected)
		glColor3f(0.0f, 1.0f, 0.0f);
	else
		glColor3f(m_outline_color.x, m_outline_color.y, m_outline_color.z);	
	glBegin(GL_LINE_STRIP);
	switch(m_agent->agent_type())
	{
	case wyc::AGENT_SLOPE_LEFT_BOTTOM:
		glVertex2f(lower.x,lower.y);
		glVertex2f(upper.x,lower.y);
		glVertex2f(lower.x,upper.y);
		glVertex2f(lower.x,lower.y);
		break;
	case wyc::AGENT_SLOPE_RIGHT_BOTTOM:
		glVertex2f(lower.x,lower.y);
		glVertex2f(upper.x,lower.y);
		glVertex2f(upper.x,upper.y);
		glVertex2f(lower.x,lower.y);
		break;
	case wyc::AGENT_SLOPE_LEFT_TOP:
		glVertex2f(lower.x,lower.y);
		glVertex2f(upper.x,upper.y);
		glVertex2f(lower.x,upper.y);
		glVertex2f(lower.x,lower.y);
		break;
	case wyc::AGENT_SLOPE_RIGHT_TOP:
		glVertex2f(upper.x,lower.y);
		glVertex2f(upper.x,upper.y);
		glVertex2f(lower.x,upper.y);
		glVertex2f(upper.x,lower.y);
		break;
	}
	glEnd();
}

//------------------------------------------------------

REG_RTTI(xplayer, xentity);

void xplayer::on_create(unsigned pid, int col_type, int col_group)
{
	xentity::on_create(pid,col_type,col_group);
	set_radius(8,8);
	m_move.set_agent(m_agent);
	m_speed.zero();
	m_gravity = -98*2.0f;
	m_walk_spd = 100;
	m_fly_spd = 100;
	m_jump_spd.set(50.0f, 160);
	set_fill(true);
	set_fill_color(0,0,0.5f);
	m_fly = false;
	m_mov_dir.zero();
}

void xplayer::stand() 
{
	m_move.stick_on_ground();
}

void xplayer::move_x(float v)
{
	m_mov_dir.x = v;
	_update_move_speed();
}

void xplayer::move_y(float v)
{
	m_mov_dir.y = v;
	_update_move_speed();
}

void xplayer::stop_x(int d)
{
	if(d<0)
	{
		if(m_mov_dir.x<0) 
		{
			m_mov_dir.x = 0;
			_update_move_speed();
		}
	}
	else if(m_mov_dir.x>0)
	{
		m_mov_dir.x = 0;
		_update_move_speed();
	}
}

void xplayer::stop_y(int d)
{
	if(d<0)
	{
		if(m_mov_dir.y<0) 
		{
			m_mov_dir.y = 0;
			_update_move_speed();
		}
	}
	else if(m_mov_dir.y>0)
	{
		m_mov_dir.y = 0;
		_update_move_speed();
	}
}


void xplayer::_update_move_speed()
{
	if(m_fly)
	{ // fly mode
		m_speed = m_mov_dir * m_fly_spd;
		return;
	}
	if(m_move.is_on_ground()) 
	{ // walk mode
		if(m_mov_dir.y>0) {
			m_speed.y = m_jump_spd.y;
			m_speed.x = m_jump_spd.x * m_mov_dir.x;
		}
		else m_speed.x = m_mov_dir.x * m_walk_spd;
		return;
	}
	// jump mode
	m_speed.x = m_mov_dir.x * m_jump_spd.x;
}

void xplayer::switch_fly_mode()
{
	m_fly = !m_fly;
	if(m_fly)
		m_move.leave_terrain();
}

void xplayer::update(double frame_time)
{
	float t = float(frame_time);
	wyc::xcollision_agent *col_obj=0;
	if(m_fly)
	{ // fly mode
		col_obj = m_move.fly(t, m_speed);
	}
	else if(m_move.is_on_ground())
	{ // walk mode
		if(m_speed.y>0) 
		{ // switch to jump
			col_obj = m_move.jump(t, m_speed, m_gravity);
		}
		else if(m_speed.x)
		{
			col_obj = m_move.walk(t,m_speed.x);
			const std::vector<float> &kpos = m_move.get_kpos();
			if(!m_move.is_on_ground()) // switch to fall
				m_speed.set(m_jump_spd.x*m_mov_dir.x,0);
		}
	}
	else // jump mode
	{
		if(m_speed.y<=0) {
			// [2013-10-18 11:10:47] [INFO] [RID:2] [24] (PID=38) move.fall (0.07822687178850174, 0.0, -83.381256103515625, -98.0)

		//	m_speed.set(0.0, -83.381256103515625f);
		//	t = 1.07822687178850174f;
			col_obj = m_move.fall(t, m_speed, -98.0f);

		//	col_obj = m_move.fall(t, m_speed, m_gravity);
			if(m_move.is_on_ground())
				m_speed.x = m_walk_spd * m_mov_dir.x;
		}
		else {
			col_obj = m_move.jump(t, m_speed, m_gravity);
		}
	}
	if(m_agent->is_dirty())
		m_agent->update_aabb();
}

}; // namespace demo

