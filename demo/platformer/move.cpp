#include "move.h"

namespace wyc
{

// 浮点误差容忍值
#define EPSILON float(1e-4)

// 求解一元二次方程: a*(x^2) + b*x + c = 0
// 返回大于0的最小解
float solve_quadratic (float a, float b, float c)
{
	if(!a) return -c/b;
	float delta = std::sqrt(b*b - 4*a*c);
	float a2 = 1.0f/(2*a);
	float t1 = (-b+delta)*a2;
	float t2 = (-b-delta)*a2;
	if(t1<0)
		return t2;
	return t1<t2 ? t1 : t2;
}

unsigned xmove::ms_terrain_filter = 0;
unsigned xmove::ms_platform_filter= 0;
unsigned xmove::ms_standon_filter = 0;
float xmove::ms_drawback = 0.05f;
float xmove::ms_stair_height = 2;
float xmove::ms_slope_can_stand = 1;
xvec2f_t xmove::ms_min_dir;
xvec2f_t xmove::ms_max_dir;

xrecorder xmove::ms_rec;

void xmove::init_move_env(unsigned terrain_filter, unsigned platform_filter)
{
	ms_terrain_filter = terrain_filter;
	ms_platform_filter = platform_filter;
	ms_standon_filter = terrain_filter | platform_filter;
	ms_min_dir.set( 0.034899f,0.999391f);
	ms_max_dir.set(-0.034899f,0.999391f);
}

xmove::xmove () : m_agent(0), m_terrain(0)
{
	m_kpos.reserve(15);
}

xcollision_agent* xmove::_detect_collision(const xvec2f_t& offset, unsigned filter, float &dt, xvec2f_t &normal, unsigned normal_filter)
{
	if(m_agent->is_dirty()) {
		m_agent->update_aabb();
	}
	xsweep_detector detector(m_agent, offset);
	wyc::xvec2f_t lower, upper, end;
	end = m_agent->get_position() + offset;
	if(offset.x>=0) {
		lower.x = m_agent->get_lower().x;
		upper.x = end.x + m_agent->get_radius().x;
	}
	else {
		lower.x = end.x - m_agent->get_radius().x;
		upper.x = m_agent->get_upper().x;
	}
	if(offset.y>=0) {
		lower.y = m_agent->get_lower().y;
		upper.y = end.y + m_agent->get_radius().y;
	}
	else {
		lower.y = end.y - m_agent->get_radius().y;
		upper.y = m_agent->get_upper().y;
	}
	if(normal_filter)
	{
		detector.set_normal_filter(normal_filter,ms_min_dir,ms_max_dir);
	}
	m_agent->parent->find_neighbors(lower,upper,filter,detector);
	xcollision_agent *collidee = detector.get_object();
	if(collidee) {
		dt = detector.get_distance();
		normal = detector.get_normal();
	}
	return collidee;
}

xcollision_agent* xmove::_detect_stand_point(xvec2f_t &pos)
{
	if(m_agent->is_dirty()) 
		m_agent->update_aabb();
	pos.set(0, -float(m_agent->parent->height()) );
	wyc::xsweep_detector detector(m_agent,pos);
	wyc::xvec2f_t lower = m_agent->lower, upper = m_agent->upper;
	lower.y += pos.y;
	m_agent->parent->find_neighbors(lower,upper,ms_standon_filter,detector);
	wyc::xcollision_agent *collidee = detector.get_object();
	if(!collidee)
		return 0;
	pos.y *= detector.get_distance();
	pos += m_agent->get_position();
	pos.y += ms_drawback;
	return collidee;
}

bool xmove::stick_on_ground()
{
	assert(m_agent->parent);
	xvec2f_t pos;
	wyc::xcollision_agent *collidee = _detect_stand_point(pos);
	if(!collidee)
		return false;
	m_agent->set_position(pos.x, pos.y);
	m_agent->update_aabb();
	set_terrain(collidee);
	return true;
}

void xmove::set_terrain(xcollision_agent *terrain)
{
	assert(terrain);
	int type = terrain->agent_type();
	const xvec2f_t &lower = terrain->get_lower(), &upper = terrain->get_upper();
	const xvec2f_t &self_radius = m_agent->get_radius();
	if(type==AGENT_SLOPE_LEFT_BOTTOM || type==AGENT_SLOPE_RIGHT_BOTTOM)
	{
		if(terrain->get_slope_normal().x>0) 
		{
			/*  |\ 
			    | \ 
			    |__\  left-bottom slope */
			m_inflections[0] = lower.x-self_radius.x-EPSILON;
			m_inflections[1] = lower.x+self_radius.x;
			m_inflections[2] = upper.x+self_radius.x+EPSILON;
			m_apex = 1;
		}
		else
		{
			/*    /|
			    /  |
			  /____|  right-bottom slope */
			m_inflections[0] = lower.x-self_radius.x-EPSILON;
			m_inflections[1] = upper.x-self_radius.x;
			m_inflections[2] = upper.x+self_radius.x+EPSILON;
			m_apex = 2;
		}
		m_cntsec = 3;
	}
	else // flat ground
	{
		m_inflections[0] = lower.x-self_radius.x-EPSILON;
		m_inflections[1] = m_inflections[2] = upper.x+self_radius.x+EPSILON;
		m_apex = -1;
		m_cntsec = 2;
	}
	for(m_cursec=0; m_cursec<m_cntsec; ++m_cursec)
	{
		if(m_agent->get_position().x < m_inflections[m_cursec])
			break;
	}
	if(m_cursec>0 && m_cursec<m_cntsec)
		m_terrain = terrain;
	else
		m_terrain = 0;
}

#define PUSH_KPOS_WALK(kpos, t, x, y) {\
	kpos.push_back(t);\
	kpos.push_back(x);\
	kpos.push_back(y);\
}

xcollision_agent* xmove::walk (float &t, float speed, unsigned extra_filter, unsigned max_iteration)
{
	m_kpos.clear();
	if(!m_terrain || !speed) {
		// not on ground
		return 0;
	}
	char d, s;
	if(speed<0)
	{
		d = -1;
		s = -1;
	}
	else
	{
		d = 1;
		s = 0;
	}
	xvec2f_t kp = m_agent->get_position();
	float spdx, spdy;
	xcollision_agent *collidee = 0;
	unsigned group_filter = ms_standon_filter | extra_filter;
	for(unsigned iteration = 0; iteration<max_iteration; ++iteration)
	{
		const xvec2f_t &terrain_normal = m_terrain->get_slope_normal();
		if(m_apex>0 && m_cursec!=m_apex) // move on slope
		{
			spdx =  terrain_normal.y * speed;
			spdy = -terrain_normal.x * speed;
		}
		else 
		{
			spdx = speed;
			spdy = 0;
		}
		xvec2f_t offset;
		offset.set(m_inflections[m_cursec+s]-m_agent->get_position().x, 0);
		float tmove = offset.x/spdx;
		xvec2f_t collide_normal;
		float dt;
		if(tmove>0)
		{
			bool in_same_section = tmove>t;
			if(in_same_section)
			{
				tmove = t;
				offset.x = tmove * spdx;
			}
			offset.y = tmove * spdy;
			collidee = _detect_collision(offset, group_filter, dt, collide_normal);
			if(!collidee)
			{
				t -= tmove;
				kp.x += offset.x;
				kp.y += offset.y;
				m_agent->set_position(kp.x,kp.y);
				PUSH_KPOS_WALK(m_kpos, tmove, kp.x, kp.y);
				if(in_same_section) {
					break;
				}
			}
		}

		if(!collidee)
		{
			m_cursec += d;
			if(m_cursec == 0 || m_cursec == m_cntsec) // leave terrain 
			{
				collidee = _detect_stand_point(kp);
				if(collidee && m_agent->get_position().y-kp.y<=ms_stair_height)
				{
					m_agent->set_position(kp.x, kp.y);
					PUSH_KPOS_WALK(m_kpos, 0, kp.x, kp.y);
					set_terrain(collidee);
				}
				else // falling
				{
					m_terrain = 0;
					collidee = 0;
					break;
				}
			}
			collidee = 0;
			continue;
		}

		tmove *= dt;
		t -= tmove;
		offset *= dt;
		kp.x += offset.x;
		kp.y += offset.y;
		
		// collide special objects
		if(collidee->get_mask() & extra_filter)
		{
			m_agent->set_position(kp.x,kp.y);
			PUSH_KPOS_WALK(m_kpos, tmove, kp.x, kp.y);
			break;
		}

		// collide terrain
		if(collide_normal.x && collide_normal.y/std::fabs(collide_normal.x)<ms_slope_can_stand)
		{ // steep slope or stair
			float stair_y = collidee->get_upper().y;
			int type = collidee->agent_type();
			if(type==AGENT_SLOPE_LEFT_BOTTOM || type==AGENT_SLOPE_RIGHT_BOTTOM) {
				const xvec2f_t &collidee_normal = collidee->get_slope_normal();
				if((kp.x-collidee->get_position().x)*collidee_normal.x>0)
					stair_y = collidee->get_lower().y;
			}
			if(stair_y - (kp.y-m_agent->get_radius().y) > ms_stair_height)
			{
				kp.x -= ms_drawback*terrain_normal.y*d;
				kp.y += ms_drawback*terrain_normal.x*d;
				m_agent->set_position(kp.x,kp.y);
				PUSH_KPOS_WALK(m_kpos, tmove, kp.x, kp.y);
				break;
			}
			PUSH_KPOS_WALK(m_kpos, tmove, kp.x, kp.y);
			kp.x += ms_drawback*d;
			kp.y  = stair_y+m_agent->get_radius().y+ms_drawback;
		}
		else
		{
			PUSH_KPOS_WALK(m_kpos, tmove, kp.x, kp.y);
			kp.y += ms_drawback;
		}
		m_agent->set_position(kp.x,kp.y);
		PUSH_KPOS_WALK(m_kpos, 0, kp.x, kp.y);
		set_terrain(collidee);
		collidee = 0;
	} // for
	m_agent->update_aabb();
	return collidee;
}

#define PUSH_KPOS_WITH_SPEED(kpos, t, x, y, spdx, spdy) {\
	size_t i = kpos.size();\
	kpos.resize(i+5);\
	kpos[i++] = t;\
	kpos[i++] = x;\
	kpos[i++] = y;\
	kpos[i++] = spdx;\
	kpos[i]   = spdy;\
}

xcollision_agent* xmove::fall (float &t, xvec2f_t &speed, float accy, unsigned extra_filter, unsigned max_iteration)
{
	m_kpos.clear();
	if( m_terrain || speed.y>0 || (speed.y==0 && accy==0) ) {
		return 0;
	}
	float tfall, tleft = 0; 
	if(accy>0)
	{
		tfall = -speed.y / accy;
		if(tfall<t)
		{
			tleft = t-tfall;
			t = tfall;
		}
	}
	xvec2f_t pos = m_agent->get_position(), offset, collide_normal;
	float dt;
	xcollision_agent *collidee = 0;
	unsigned group_filter = ms_standon_filter | extra_filter;
	bool is_on_ground = false;
	for(unsigned iteration = 0; iteration<max_iteration; ++iteration)
	{
		offset.x = speed.x * t;
		offset.y = speed.y * t + 0.5f * accy * t * t;

		collidee = _detect_collision(offset,group_filter,dt,collide_normal,ms_platform_filter);
		if(collidee)
		{ // hit something
			offset.x *= dt;
			offset.y *= dt;
			pos += offset;
			tfall = solve_quadratic(0.5f*accy, speed.y, -offset.y);
			t -= tfall;
			speed.y += accy * tfall;
			if(collidee->get_mask() & ms_standon_filter)
			{ // hit terrain
				speed.x = 0;
				if(collide_normal.y < EPSILON)
				{
					pos.x += ms_drawback * collide_normal.x;
					pos.y += ms_drawback * collide_normal.y;
					m_agent->set_position(pos.x, pos.y);
					PUSH_KPOS_WITH_SPEED(m_kpos, tfall, pos.x, pos.y, speed.x, speed.y);
					continue;
				}
				// fall on ground
				pos.y += ms_drawback;
				speed.y= 0;
				is_on_ground = true;
			}
		}
		else 
		{
			speed.y += accy * t;
			pos += offset;
			tfall = t;
			t = 0;
		}
		m_agent->set_position(pos.x,pos.y);
		if(is_on_ground) 
			set_terrain(collidee);
		PUSH_KPOS_WITH_SPEED(m_kpos, tfall, pos.x, pos.y, speed.x, speed.y);
		break;
	} // for
	t += tleft;
	m_agent->update_aabb();
	return collidee;
}

xcollision_agent* xmove::jump (float &t, xvec2f_t &speed, float accy, unsigned extra_filter, unsigned max_iteration)
{
	m_kpos.clear();
	if(speed.y<0 || (speed.y==0 && accy<=0))
		return 0;
	float tjump, tleft = 0; 
	if(accy<0)
	{
		tjump = -speed.y / accy;
		if(tjump<t)
		{
			tleft = t-tjump;
			t = tjump;
		}
	}
	xvec2f_t offset, collide_normal;
	xcollision_agent *collidee = 0;
	float dt;
	unsigned group_filter = ms_terrain_filter | extra_filter;
	// leave ground
	m_terrain = 0;
	for(unsigned iteration = 0; iteration<max_iteration; ++iteration)
	{
		offset.x = speed.x * t;
		offset.y = speed.y * t + 0.5f * accy * t * t;
		collidee = _detect_collision(offset, group_filter, dt, collide_normal);
		xvec2f_t pos = m_agent->get_position();
		if(collidee)
		{
			offset *= dt;
			pos += offset;
			tjump = solve_quadratic(0.5f*accy, speed.y, -offset.y); 
			t -= tjump;
			if(!(collidee->get_mask() & extra_filter))
			{
				pos.x += collide_normal.x * ms_drawback;
				pos.y += collide_normal.y * ms_drawback;
				if(std::fabs(collide_normal.x)>EPSILON)
					speed.x = 0;
				if(std::fabs(collide_normal.y)<EPSILON){
					speed.y += tjump*accy;
					m_agent->set_position(pos.x, pos.y);
					PUSH_KPOS_WITH_SPEED(m_kpos, tjump, pos.x, pos.y, speed.x, speed.y);
					continue;
				}
				speed.y = 0;
			}
		} 
		else
		{
			pos += offset;
			speed.y = tleft>0 ? 0 : speed.y+accy*t;
			tjump = t;
			t = 0;
		}
		m_agent->set_position(pos.x, pos.y);
		PUSH_KPOS_WITH_SPEED(m_kpos, tjump, pos.x, pos.y, speed.x, speed.y);
		break;
	} // for
	t += tleft;
	m_agent->update_aabb();
	return collidee;
}

xcollision_agent* xmove::fly  (float &t, const xvec2f_t speed, unsigned extra_filter)
{
	// leave ground
	m_terrain = 0;
	xvec2f_t pos = m_agent->get_position(), offset = speed * t, collide_normal;
	float dt;
	xcollision_agent *collidee = _detect_collision(offset, ms_terrain_filter+extra_filter, dt, collide_normal);
	if(collidee)
	{
		t -= t*dt;
		offset *= dt;
		if(collidee->get_mask() & ms_terrain_filter)
		{
			offset.x += collide_normal.x * ms_drawback;
			offset.y += collide_normal.y * ms_drawback;
		}
	}
	else
	{
		t = 0;
		pos += offset;
	}
	m_agent->set_position(pos.x, pos.y);
	m_agent->update_aabb();
	return collidee;
}

} // namespace wyc