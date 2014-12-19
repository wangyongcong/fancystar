#ifndef __HEADER_WYC_COLLISION_DETECTOR
#define __HEADER_WYC_COLLISION_DETECTOR

#include <cassert>
#include <functional>
#include "quad_tree.h"
#include "recorder.h"

namespace wyc
{

enum AGENT_TYPE
{
	AGENT_AABB = 0,
	// 4种直角斜面,LEFT_BOTTOM表示直角在左下,以此类推
	AGENT_SLOPE_LEFT_BOTTOM,
	AGENT_SLOPE_LEFT_TOP,
	AGENT_SLOPE_RIGHT_TOP,
	AGENT_SLOPE_RIGHT_BOTTOM,
	// 碰撞体类型数量
	AGENT_TYPE_COUNT,
};

#define IS_SLOPE(t) ((t)>=wyc::AGENT_SLOPE_LEFT_BOTTOM && (t)<=wyc::AGENT_SLOPE_RIGHT_BOTTOM)
#define GET_SLOPE_TYPE(t) ((t)-wyc::AGENT_SLOPE_LEFT_BOTTOM)

typedef wyc::xquad_tree::entity_t xbounding_box;

class xcollision_agent : public xbounding_box
{
protected:
	int m_agent_type;
	enum {
		IS_SLOPE	= 1,

		DIRTY_POSITION	= 0x10,
		DIRTY_RADIUS	= 0x20,
		DIRTY_AABB		= DIRTY_POSITION | DIRTY_RADIUS,
		CLEAN_DIRTY = ~(DIRTY_POSITION | DIRTY_RADIUS),
		
		MASK_SLOPE_DIRTY = IS_SLOPE | DIRTY_RADIUS, 
	};
	int m_flag;
	xvec2f_t m_center, m_radius;
	xvec2f_t m_slope; 
public:
	xcollision_agent(int type=AGENT_AABB) : xbounding_box(), m_agent_type(type)
	{
		m_center.zero();
		m_radius.zero();
		m_slope.set(0,1);
		if(IS_SLOPE(m_agent_type))
			m_flag = IS_SLOPE;
		else
			m_flag = 0;
	}
	void update_aabb()
	{
		this->lower.set(m_center.x-m_radius.x,m_center.y-m_radius.y);
		this->upper.set(m_center.x+m_radius.x,m_center.y+m_radius.y);
		if(this->parent)
			this->parent->update_entity(this);
		if(MASK_SLOPE_DIRTY==(m_flag & MASK_SLOPE_DIRTY))
			_update_slope();
		m_flag &= CLEAN_DIRTY;
	}
	inline void set_agent_type(AGENT_TYPE t) 
	{
		m_agent_type = t;
		if(IS_SLOPE(m_agent_type))
			m_flag |= IS_SLOPE | DIRTY_RADIUS; 
	}
	inline int agent_type() const 
	{
		return m_agent_type;
	}
	inline void set_position (float x, float y)
	{
		m_center.set(x,y);
		m_flag |= DIRTY_POSITION;
	}
	inline void set_position (const xvec2f_t &pos)
	{
		m_center = pos;
		m_flag |= DIRTY_POSITION;
	}
	inline const xvec2f_t& get_position() const 
	{
		return m_center;
	}
	inline void set_radius(float half_w, float half_h)
	{
		m_radius.set(half_w,half_h);
		assert(m_radius.x>=0 && m_radius.y>=0);
		m_flag |= DIRTY_RADIUS;
	}
	inline void set_radius(const xvec2f_t &radius)
	{
		m_radius = radius;
		assert(m_radius.x>=0 && m_radius.y>=0);
		m_flag |= DIRTY_RADIUS;
	}
	inline const xvec2f_t& get_radius() const
	{
		return m_radius;
	}
	inline const xvec2f_t& get_lower() const
	{
		return this->lower;
	}
	inline const xvec2f_t& get_upper() const
	{
		return this->upper;
	}
	inline void set_mask (unsigned mask) 
	{
		this->mask=mask;
	}
	inline int get_mask() const 
	{
		return this->mask;
	}
	inline void set_data (void *d)
	{
		this->data = d;
	}
	inline void* get_data() const 
	{
		return this->data;
	}
	inline const float* get_vertices() const 
	{
		return this->verts;
	}
	inline bool is_dirty() const 
	{
		return 0!=(m_flag & DIRTY_AABB);
	}
	inline const xvec2f_t& get_slope_normal() const 
	{
		return m_slope;
	}
protected:
	void _update_slope();
};

//------------------------------------------------------------------

class xpoint_detector
{
	xvec2f_t m_position;
	xcollision_agent *m_picked;
public:
	xpoint_detector(float x, float y);
	void operator() (xbounding_box *en);
	inline xcollision_agent* get_object() const
	{
		return m_picked;
	}
};

class xstatic_detector
{
	xcollision_agent *m_self;
	std::vector<xcollision_agent*> m_list;
	bool m_is_aabb;
public:
	xstatic_detector(xcollision_agent *agent);
	void operator() (xbounding_box *en);
	inline unsigned count() const {
		return m_list.size();
	}
	inline xcollision_agent* operator[] (int idx) {
		return m_list[idx];
	}
};

class xsweep_detector
{
	xcollision_agent *m_self, *m_collided_obj;
	xvec2f_t m_offset, m_collide_normal;
	xvec2f_t m_min_dir, m_max_dir;
	float m_distance;
	unsigned m_normal_filter;
public:
	xsweep_detector(xcollision_agent *agent, const wyc::xvec2f_t &offset);
	void operator() (xbounding_box *en);
	xcollision_agent* get_object() const 
	{
		return m_collided_obj;
	}
	float get_distance() const
	{
		return m_distance;
	}
	const wyc::xvec2f_t& get_normal() const
	{
		return m_collide_normal;
	}
	void set_normal_filter(unsigned normal_filter, const wyc::xvec2f_t &min_dir, const wyc::xvec2f_t &max_dir)
	{
		m_min_dir = min_dir;
		m_max_dir = max_dir;
		m_normal_filter = normal_filter;
	}
	void clear_normal_filter()
	{
		m_normal_filter = 0;
	}
};

class xsweep_intersect_detector
{
	xcollision_agent *m_self;
	typedef std::pair<float,xcollision_agent*> elem_t;
	typedef std::vector<elem_t> list_t;
	list_t m_list;
	xvec2f_t m_offset, m_surface, m_my_boundary;
public:
	xsweep_intersect_detector(xcollision_agent *agent, const wyc::xvec2f_t &offset);
	void operator() (xbounding_box *en);
	inline unsigned count() const {
		return m_list.size();
	}
	inline xcollision_agent* operator[] (int idx) {
		return m_list[idx].second;
	}
private:
	void _insert(float t, xcollision_agent *agent);

	struct _greater
		: public std::unary_function<elem_t, bool>
	{	// functor for operator<
		float m_val;
		_greater(float t) : m_val(t) {
		}
	bool operator()(const elem_t& v) const
		{	// apply operator< to operands
			return (v.first > m_val);
		}
	};

};

}; // namespace wyc

#endif // __HEADER_WYC_COLLISION_DETECTOR

