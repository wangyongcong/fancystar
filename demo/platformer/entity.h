#ifndef __HEADER_DEMO_ENTITY
#define __HEADER_DEMO_ENTITY

#include "wyc/obj/object.h"
#include "wyc/math/vecmath.h"
#include "wyc/render/renderer.h"

#include "collision_detector.h"
#include "move.h"

namespace demo
{

enum ENTITY_GROUP
{
	CG_USED	   = 0,	// 不再发生碰撞
	CG_TERRAIN = 1,	// 基础地形
	CG_OBSTACLE= 2,	// 动态障碍物
	CG_MONSTER = 4,	// 怪物
	CG_DESTRUCTIBLE = 8,	// 可破坏场景物件
	CG_PLAYER_BULLET  = 16, // 玩家子弹
	CG_MONSTER_BULLET = 32, // 怪物子弹
	CG_BULLET = CG_PLAYER_BULLET | CG_MONSTER_BULLET,
	CG_PLATFORM = 64,	// 平台
	CG_ITEM = 128,		// 掉落物
	CG_PLAYER_BEG = 256, // 玩家,Start
	CG_PLAYER_T1 = 256,	// 玩家,Team1
	CG_PLAYER_T2 = 512,	// 玩家,Team2
	CG_PLAYER = CG_PLAYER_T1 | CG_PLAYER_T2,	// 所有玩家
	CG_PLAYER_END = 512, // 玩家,End

	CG_FILTER_ALL = 0xFFFF, // 全部的group
};

class xentity : public wyc::xobject
{
	USE_RTTI;
protected:
	unsigned m_pid;
	wyc::xcollision_agent *m_agent;
	wyc::xvec3f_t m_outline_color, m_fill_color;
	bool m_selected, m_fill;
public:
	xentity();
	virtual void on_create(unsigned pid, int col_type, int col_group);
	virtual void on_destroy();
	virtual void draw(wyc::xrenderer *rd);
	inline unsigned pid() const 
	{
		return m_pid;
	}
	void set_position(float x, float y)
	{
		m_agent->set_position(x,y);
	}
	void set_position(const wyc::xvec2f_t &pos)
	{
		m_agent->set_position(pos);
	}
	const wyc::xvec2f_t& get_position() const
	{
		return m_agent->get_position();
	}
	void set_radius(float half_w, float half_h)
	{
		m_agent->set_radius(half_w,half_h);
	}
	void set_radius(const wyc::xvec2f_t &r)
	{
		m_agent->set_radius(r);
	}
	const wyc::xvec2f_t& get_radius() const
	{
		return m_agent->get_radius();
	}
	void set_aabb(const wyc::xvec2f_t &lower, const wyc::xvec2f_t &upper)
	{
		wyc::xvec2f_t pos=(lower+upper)*0.5f, r= (upper-lower)*0.5f;
		m_agent->set_position(pos.x,pos.y);
		m_agent->set_radius(r.x,r.y);
	}
	void update_aabb()
	{
		if(m_agent->is_dirty())
			m_agent->update_aabb();
	}
	void set_outline_color(float r, float g, float b)
	{
		m_outline_color.set(r,g,b);
	}
	void set_fill_color(float r, float g, float b)
	{
		m_fill_color.set(r,g,b);
	}
	void set_fill (bool b)
	{
		m_fill = b;
	}
	wyc::xcollision_agent* get_agent() 
	{
		return m_agent;
	}
	void set_selected(bool b)
	{
		m_selected=b;
	}
	int type() const 
	{
		return m_agent->agent_type();
	}
	int group() const 
	{
		return m_agent->get_mask();
	}
	void set_type (wyc::AGENT_TYPE type)
	{
		m_agent->set_agent_type(type);
	}
};

class xobstacle : public xentity
{
	USE_RTTI;
public:
	virtual void draw(wyc::xrenderer *rd);
	const wyc::xvec2f_t& get_slope_normal() const
	{
		return m_agent->get_slope_normal();
	}
private:
	void _draw_slope(wyc::xrenderer *rd);
};

class xplayer : public xentity
{
	USE_RTTI;
	wyc::xmove m_move;
	unsigned m_state;
	float m_walk_spd, m_gravity, m_fly_spd;
	wyc::xvec2f_t m_jump_spd;
	wyc::xvec2f_t m_speed;
	wyc::xvec2f_t m_mov_dir;
	bool m_fly;
public:
	virtual void on_create(unsigned pid, int col_type, int col_group);
	void stand();
	void update(double frame_time);
	void stop();
	void switch_fly_mode();
	void move_x(float v);
	void move_y(float v);
	void stop_x(int d);
	void stop_y(int d);
private:
	void _update_move_speed();
};

};

#endif // __HEADER_DEMO_ENTITY