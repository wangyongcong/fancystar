#include "collision_detector.h"
#include <algorithm>

namespace wyc
{

static int s_slope2x[4] =
{
	0, // SLOPE_LEFT_BOTTOM, 0100
	0, // SLOPE_LEFT_TOP, 1100
	2, // SLOPE_RIGHT_TOP, 1110
	2, // SLOPE_RIGHT_BOTTOM, 0110
};

static int s_slope2y[4] =
{
	1, // SLOPE_LEFT_BOTTOM, 0100
	3, // SLOPE_LEFT_TOP, 1100
	3, // SLOPE_RIGHT_TOP, 1110
	1, // SLOPE_RIGHT_BOTTOM, 0110
};

void xcollision_agent::_update_slope()
{
	switch(m_agent_type)
	{
	case AGENT_SLOPE_LEFT_BOTTOM:
		m_slope.set(this->upper.y-this->lower.y,this->upper.x-this->lower.x);
		break;
	case AGENT_SLOPE_LEFT_TOP:
		m_slope.set(this->upper.y-this->lower.y,this->lower.x-this->upper.x);
		break;
	case AGENT_SLOPE_RIGHT_TOP:
		m_slope.set(this->lower.y-this->upper.y,this->lower.x-this->upper.x);
		break;
	case AGENT_SLOPE_RIGHT_BOTTOM:
		m_slope.set(this->lower.y-this->upper.y,this->upper.x-this->lower.x);
		break;
	default:
		m_slope.set(0,1);
		return;
	};
	m_slope.normalize();
}

//--------------------------------------------------------------------------------------
// point in geometry detector
//

typedef bool (*pfn_is_point_in) (const xcollision_agent *agent, float x, float y);

bool is_point_in_aabb (const xcollision_agent *agent, float x, float y)
{
	return x>=agent->get_lower().x && x<=agent->get_upper().x && y>=agent->get_lower().y && y<=agent->get_upper().y;
}

bool is_point_in_slope(const xcollision_agent *agent, float x, float y)
{
	static float ls_xsign[4]={1,1,-1,-1}, ls_ysign[4]={1,-1,-1,1};
	xvec2f_t size = agent->get_radius();
	size.scale(2);
	if(size.x==0)
		return x==agent->get_position().x && y>=agent->get_lower().y && y<=agent->get_upper().y;
	if(size.y==0)
		return y==agent->get_position().y && x>=agent->get_lower().x && x<=agent->get_upper().x;
	int slope_type = GET_SLOPE_TYPE(agent->agent_type());
	const float *verts=agent->get_vertices();
	wyc::xvec2f_t org;
	org.set( x-verts[s_slope2x[slope_type]], y-verts[s_slope2y[slope_type]] );
	org.x *= ls_xsign[slope_type];
	org.y *= ls_ysign[slope_type];
	float s=org.x/size.x;
	float t=org.y/size.y;
	return s>=0 && t>=0 && s+t<=1;
}

static pfn_is_point_in s_point_in_functions[AGENT_TYPE_COUNT] =
{
	(pfn_is_point_in)&is_point_in_aabb,
	(pfn_is_point_in)&is_point_in_slope,
	(pfn_is_point_in)&is_point_in_slope,
	(pfn_is_point_in)&is_point_in_slope,
	(pfn_is_point_in)&is_point_in_slope,
};

//--------------------------------------------------------------------------------------
// geometry overlap detector
//

typedef bool (*pfn_is_overlap) (const xcollision_agent *first, const xcollision_agent *second);

void project_on_slope_normal (const xcollision_agent *aabb, const xcollision_agent *slope, float &boundary_n, float &lower_n, float &upper_n)
{
	const xvec2f_t &org = slope->get_position();
	int slope_type = GET_SLOPE_TYPE(slope->agent_type());
	const float *verts= slope->get_vertices();
	xvec2f_t v; 
	v.set( verts[s_slope2x[slope_type]], verts[s_slope2y[slope_type]] );
	v -= org;
	boundary_n=v.dot(slope->get_slope_normal());
	const xvec2f_t &lower = aabb->get_lower(), &upper = aabb->get_upper();
	float t1=(lower-org).dot(slope->get_slope_normal());
	v.set(lower.x,upper.y);
	float t2=(v-org).dot(slope->get_slope_normal());
	float t3=(upper-org).dot(slope->get_slope_normal());
	v.set(upper.x,lower.y);
	float t4=(v-org).dot(slope->get_slope_normal());
	if(t2<t1)
		std::swap(t2,t1);
	if(t4<t3)
		std::swap(t3,t4);
	lower_n = t1>t3?t3:t1;
	upper_n = t4<t2?t2:t4;
}

bool is_overlap_aabb_aabb (const xcollision_agent *first, const xcollision_agent *second)
{
	return !(first->get_lower().x>second->get_upper().x || first->get_upper().x<second->get_lower().x \
		|| first->get_lower().y>second->get_upper().y || first->get_upper().y<second->get_lower().y);
}

bool is_overlap_aabb_slope (const xcollision_agent *first, const xcollision_agent *second)
{
	assert(IS_SLOPE(second->agent_type()));
	float boundary_n, lower_n, upper_n;
	project_on_slope_normal(first,second,boundary_n,lower_n,upper_n);
	return lower_n<=0 && upper_n>=boundary_n;
}

bool is_overlap_slope_slope (const xcollision_agent *, const xcollision_agent *)
{
	return false;
}

pfn_is_overlap s_aabb_overlap_functions[AGENT_TYPE_COUNT]=
{
	(pfn_is_overlap)&is_overlap_aabb_aabb,
	(pfn_is_overlap)&is_overlap_aabb_slope,
	(pfn_is_overlap)&is_overlap_aabb_slope,
	(pfn_is_overlap)&is_overlap_aabb_slope,
	(pfn_is_overlap)&is_overlap_aabb_slope,
};

pfn_is_overlap s_slope_overlap_functions[AGENT_TYPE_COUNT]=
{
	0,
	(pfn_is_overlap)&is_overlap_slope_slope,
	(pfn_is_overlap)&is_overlap_slope_slope,
	(pfn_is_overlap)&is_overlap_slope_slope,
	(pfn_is_overlap)&is_overlap_slope_slope,
};

static pfn_is_overlap* s_overlap_functions[AGENT_TYPE_COUNT] =
{
	s_aabb_overlap_functions,
	s_slope_overlap_functions,
	s_slope_overlap_functions,
	s_slope_overlap_functions,
	s_slope_overlap_functions,
};

//--------------------------------------------------------------------------------------
// geometry sweep detector
//

typedef float (*pfn_sweep) (const xcollision_agent *first, const xcollision_agent *second, const xvec2f_t &offset, xvec2f_t &col_normal);

float sweep_aabb_aabb(const xcollision_agent *first, const xcollision_agent *second, const xvec2f_t &offset, xvec2f_t &col_normal)
{
	float dist, t=0, tx=0, ty=0, nx=0, ny=0;
	// project onto x-axis
	if(offset.x!=0)
	{
		if(offset.x>0) 
			dist = second->get_lower().x-first->get_upper().x;
		else 
			dist = second->get_upper().x-first->get_lower().x;
		tx = dist/offset.x;
		if(tx>t && tx<1.0001f) { // collide on left/right edge
			t=tx;
			nx = offset.x>0?-1.0f:1.0f;
		}
	}
	// project onto y-axis
	if(offset.y!=0)
	{
		if(offset.y>0)
			dist = second->get_lower().y-first->get_upper().y;
		else
			dist = second->get_upper().y-first->get_lower().y;
		ty = dist/offset.y;
		if(ty>t && ty<1.0001f) {
			t=ty;
			nx = 0;
			ny = offset.y>0?-1.0f:1.0f;
		}
	}
	if(t>0)
	{
		dist = offset.x * t;
		if( first->get_lower().x+dist > second->get_upper().x+EPSILON_E4 
			|| first->get_upper().x+dist < second->get_lower().x-EPSILON_E4 )
			return 0;
		dist = offset.y * t;
		if( first->get_lower().y+dist > second->get_upper().y+EPSILON_E4 
			|| first->get_upper().y+dist < second->get_lower().y-EPSILON_E4 )
			return 0;
		if(tx==ty)
			col_normal.set(0.7071f,0.7071f);
		else
			col_normal.set(nx,ny);
	}
	return t;
}

float sweep_aabb_slope(const xcollision_agent *first, const xcollision_agent *second, const xvec2f_t &offset, xvec2f_t &col_normal)
{
	float dist, t=0, tx=0, ty=0, tz=0;
	// project onto x-axis
	if(offset.x!=0)
	{
		if(offset.x>0) 
			dist = second->get_lower().x-first->get_upper().x;
		else 
			dist = second->get_upper().x-first->get_lower().x;
		tx = dist/offset.x;
		if(tx>t && tx<1.0001f) { // collide on left/right edge
			t = tx;
			col_normal.set(offset.x>0?-1.0f:1.0f,0);
		}
	}
	// project onto y-axis
	if(offset.y!=0)
	{
		if(offset.y>0)
			dist = second->get_lower().y-first->get_upper().y;
		else
			dist = second->get_upper().y-first->get_lower().y;
		ty = dist/offset.y;
		if(ty>t && ty<1.0001f) {
			t = ty;
			col_normal.set(0,offset.y>0?-1.0f:1.0f);
		}
		else if(ty>0 && ty==tx)
			col_normal.set(0.7071f,0.7071f);
	}
	// project onto slope normal
	float offset_z = offset.dot( second->get_slope_normal() );
	float slope_lower, lower, upper;
	if(offset_z) {
		project_on_slope_normal(first,second,slope_lower,lower,upper);
		if(offset_z<0)
			dist = 0-lower;
		else
			dist = slope_lower-upper;
		tz = dist/offset_z;
		if(tz>t && tz<1.0001f) {
			t = tz;
			col_normal = second->get_slope_normal();
		}
	}
	if(t>0)
	{
		dist = offset.x * t;
		if( first->get_lower().x+dist > second->get_upper().x+EPSILON_E4 
			|| first->get_upper().x+dist < second->get_lower().x-EPSILON_E4 )
			return 0;
		dist = offset.y * t;
		if( first->get_lower().y+dist > second->get_upper().y+EPSILON_E4 
			|| first->get_upper().y+dist < second->get_lower().y-EPSILON_E4 )
			return 0;
		if(offset_z)
			dist = offset_z * t;
		else {
			dist = 0;
			project_on_slope_normal(first,second,slope_lower,lower,upper);
		}
		if(lower+dist > EPSILON_E4 || upper+dist < slope_lower-EPSILON_E4)
			return 0;
	}
	return t;
}

pfn_sweep s_aabb_sweep_functions[AGENT_TYPE_COUNT]=
{
	(pfn_sweep)&sweep_aabb_aabb,
	(pfn_sweep)&sweep_aabb_slope,
	(pfn_sweep)&sweep_aabb_slope,
	(pfn_sweep)&sweep_aabb_slope,
	(pfn_sweep)&sweep_aabb_slope,
};

pfn_sweep s_slope_sweep_functions[AGENT_TYPE_COUNT]=
{
	0,
	0,
	0,
	0,
	0,
};

static pfn_sweep* s_sweep_functions[AGENT_TYPE_COUNT] =
{
	s_aabb_sweep_functions,
	s_slope_sweep_functions,
	s_slope_sweep_functions,
	s_slope_sweep_functions,
	s_slope_sweep_functions,
};

//--------------------------------------------------------------------------------------

xpoint_detector::xpoint_detector(float x, float y)
{
	m_position.set(x,y);
	m_picked = 0;
}

void xpoint_detector::operator() (xbounding_box *en)
{
	xcollision_agent *agent = (xcollision_agent*)en;
	if(agent->agent_type()!=AGENT_AABB)
	{
		pfn_is_point_in is_point_in = s_point_in_functions[agent->agent_type()];
		if(0==is_point_in || !is_point_in(agent,m_position.x,m_position.y))
			return;
	}
	m_picked = agent;
}

//--------------------------------------------------------------------------------------

xstatic_detector::xstatic_detector(xcollision_agent *agent) : m_self(agent)
{
	m_is_aabb = m_self->agent_type()==AGENT_AABB;
}

void xstatic_detector::operator() (xbounding_box *en)
{
	if(m_self==en)
		return;
	xcollision_agent *first=m_self, *second=(xcollision_agent*)en;
	if(m_is_aabb && AGENT_AABB==second->agent_type())
	{
		m_list.push_back(second);
		return;
	}
	if(first->agent_type()>second->agent_type())
		std::swap(first,second);
	pfn_is_overlap is_overlap = s_overlap_functions[first->agent_type()][second->agent_type()];
	if(is_overlap && is_overlap(first,second))
		m_list.push_back((xcollision_agent*)en);
}

//--------------------------------------------------------------------------------------

xsweep_detector::xsweep_detector(xcollision_agent *agent, const xvec2f_t &offset)
{
	m_self = agent;
	m_collided_obj = 0;
	m_offset = offset;
	m_distance = 1.0001f; // 1.0f + EPSILON_E4
	m_normal_filter = 0;
}

void xsweep_detector::operator() (xbounding_box *en)
{
	if(m_self == en)
		return;
	xcollision_agent *first=m_self, *second=(xcollision_agent*)en;
	if(first->agent_type()>second->agent_type())
		std::swap(first,second);
	pfn_sweep sweep = s_sweep_functions[first->agent_type()][second->agent_type()];
	if(sweep)
	{
		xvec2f_t col_normal;
		float t = sweep(first,second,m_offset,col_normal);
		if(t>0 && t<m_distance) {
			// 对碰撞方向进行过滤
			if(m_normal_filter&en->mask && !(m_min_dir.cross(col_normal)>=0 && col_normal.cross(m_max_dir)>=0))
				return;
			m_collided_obj = (xcollision_agent*)en;
			m_collide_normal = col_normal;
			m_distance = t;
		}
	}
}

//--------------------------------------------------------------------------------------

xvec2f_t project_on_surface_aabb(const xvec2f_t &surface, const xcollision_agent *agent)
{
	xvec2f_t boundary;
	boundary.set(0,0);
	const xvec2f_t& r = agent->get_radius();
	xvec2f_t verts[4];
	verts[0].set(-r.x,-r.y);
	verts[1].set( r.x,-r.y);
	verts[2].set( r.x, r.y);
	verts[3].set(-r.x, r.y);
	float v;
	for (int i=0; i<4; ++i)
	{
		v = verts[i].dot(surface);
		if(v<boundary.x)
			boundary.x = v;
		if(v>boundary.y)
			boundary.y = v;
	}
	return boundary;
}

xvec2f_t project_on_surface_slope(const xvec2f_t &surface, const xcollision_agent *agent)
{
	xvec2f_t boundary;
	boundary.set(0,0);
	xvec2f_t verts[3];
	const xvec2f_t& r = agent->get_radius();
	switch(agent->agent_type())
	{
	case AGENT_SLOPE_LEFT_BOTTOM:
		verts[0].set(-r.x,-r.y);
		verts[1].set( r.x,-r.y);
		verts[2].set(-r.x, r.y);
		break;
	case AGENT_SLOPE_LEFT_TOP:
		verts[0].set(-r.x, r.y);
		verts[1].set( r.x, r.y);
		verts[2].set(-r.x,-r.y);
		break;
	case AGENT_SLOPE_RIGHT_TOP:
		verts[0].set( r.x, r.y);
		verts[1].set( r.x,-r.y);
		verts[2].set(-r.x, r.y);
		break;
	case AGENT_SLOPE_RIGHT_BOTTOM:
		verts[0].set( r.x,-r.y);
		verts[1].set(-r.x,-r.y);
		verts[2].set( r.x, r.y);
		break;
	default:
		return boundary;
	}
	float v;
	for(int i=0; i<3; ++i)
	{
		v = verts[i].dot(surface);
		if(v<boundary.x)
			boundary.x = v;
		if(v>boundary.y)
			boundary.y = v;
	}
	return boundary;
}

xsweep_intersect_detector::xsweep_intersect_detector(xcollision_agent *agent, const wyc::xvec2f_t &offset)
{
	m_self = agent;
	m_offset = offset;
	m_surface.set(offset.y, -offset.x);
	m_surface.normalize();
	if(IS_SLOPE(agent->agent_type()))
		m_my_boundary = project_on_surface_slope(m_surface, agent);
	else
		m_my_boundary = project_on_surface_aabb(m_surface, agent);
}

void xsweep_intersect_detector::operator() (xbounding_box *en)
{
	if(m_self == en)
		return;
	xcollision_agent *first=m_self, *second=(xcollision_agent*)en;
	if(first->agent_type()>second->agent_type())
		std::swap(first,second);
	pfn_sweep sweep = s_sweep_functions[first->agent_type()][second->agent_type()];
	if(sweep)
	{
		xvec2f_t col_normal;
		float t = sweep(first,second,m_offset,col_normal);
		if(t>0) _insert(t,(xcollision_agent*)en);
	}
}

void xsweep_intersect_detector::_insert(float t, xcollision_agent *agent)
{
	// 按t值从小到大插入
	list_t::iterator iter = std::find_if(m_list.begin(),m_list.end(),_greater(t));
	m_list.insert(iter,std::pair<float,xcollision_agent*>(t,agent));
}

}; // namespace wyc
