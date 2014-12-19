#ifndef __HEADER_WYC_OBJ_BASE
#define __HEADER_WYC_OBJ_BASE

#include <cassert>
#include "wyc/util/time.h"
#include "wyc/obj/object.h"
#include "wyc/math/transform.h"
#include "wyc/render/renderer.h"

#include "aoi_mgr.h"
#include "scope.h"

using namespace wyc;

struct xaoi_stat
{
	double total_move_time;

	unsigned m_frame_count;
	
	xaoi_stat()
	{
		reset();
	}
	void update()
	{
		m_frame_count+=1;
	}
	void reset()
	{
		total_move_time=0;
		m_frame_count=1;
	}

	double ms_per_frame()
	{
		return total_move_time/m_frame_count;
	}
};

extern xaoi_stat g_aoi_stat;

class xbase_object : public xobject
{
	USE_RTTI;
protected:
	xtransform m_transform;
	xaoi_entity *m_aoi_entity;
	xpointer<xscope> m_scope;
	xvec3f_t m_color;
	xmat4f_t m_prev_trans;
public:
	xbase_object ()
	{
		m_aoi_entity = 0;
		m_color.set(1,1,1);
		m_prev_trans.identity();
	}

	virtual void on_destroy()
	{
		if(m_aoi_entity)
		{
			aoi_entity_destroy(m_aoi_entity);
			m_aoi_entity=0;
		}
		m_scope=0;
	}

	xtransform* get_transform()
	{
		return &m_transform;
	}

	void interpolate_world2local_matrix(xmat4f_t &ret, float delta)
	{
		xmat4f_t mat1 = m_prev_trans, mat2=m_transform.world2local();
		mat1.scale(delta);
		mat2.scale(1-delta);
		ret=mat1+mat2;
	}

	virtual void update_transform()
	{
		bool is_moved = m_transform.is_moved();
		m_prev_trans = m_transform.world2local();
		m_transform.update();
		if(is_moved && m_aoi_entity)
		{
			double t = wyc::xtime_source::singleton().get_time();
			aoi_entity_move(m_aoi_entity,m_transform.position());
			t = wyc::xtime_source::singleton().get_time() - t;
			g_aoi_stat.total_move_time+=t;
		}
	}

	virtual void draw( const xshader *sh )
	{
	}

	void draw_scope( const xshader *sh )
	{
		if(!m_scope)
			return;
		GLint uf=sh->get_uniform("position");
		if(-1==uf)
			return;
		const xvec3f_t &pos = m_transform.position();
		glUniform3f(uf,pos.x,pos.y,pos.z);
		uf=sh->get_uniform("color");
		if(-1==uf)
			return;
		glUniform3f(uf,m_color.x,m_color.y,m_color.z);
		m_scope->draw();
	}

	virtual void join_aoi (xaoi_manager *mgr, float radius, unsigned group, unsigned group_filter)
	{
		if(m_aoi_entity)
		{
			wyc_warn("xbase_object::join_aoi: object is already in AOI manager");
			return;
		}
		m_aoi_entity = aoi_entity_create(this,radius,group,group_filter);
		aoi_entity_join(mgr,m_aoi_entity,m_transform.position());
		on_join_aoi();
	}

	virtual void on_join_aoi()
	{
	}

	virtual void aoi_on_enter(xbase_object *obj)
	{
		wyc_print("[%s] enter my(%s) aoi",obj->my_class()->name,my_class()->name);
	}

	virtual void aoi_on_leave(xbase_object *obj)
	{
		wyc_print("[%s] leave my(%s) aoi",obj->my_class()->name,my_class()->name);
	}

};

#endif // __HEADER_WYC_OBJ_BASE

