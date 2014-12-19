#ifndef __HEADER_WYC_SIMPLE_CAMERA
#define __HEADER_WYC_SIMPLE_CAMERA

#include "wyc/obj/object.h"
#include "wyc/math/vecmath.h"
#include "wyc/math/transform.h"

class xsimple_camera : public wyc::xobject
{
	USE_RTTI;
	wyc::xtransform m_transform;
	wyc::xspherical<float> m_position;
	float m_min_dist, m_max_dist;
	wyc::xmat4f_t m_proj;
	bool m_change_pos;
public:
	xsimple_camera() 
	{
		m_position.latitude=0;
		m_position.longitude=0;
		m_position.r=1.0f;
		m_min_dist = 1.0f;
		m_max_dist = 4.0f;
		m_proj.identity();
		m_change_pos=true;
	}

	void set_projection(float fov, float aspect, float fnear, float ffar)
	{
		set_perspective(m_proj,fov,aspect,fnear,ffar);
	}

	void set_position(float latitude, float longitude, float radius)
	{
		m_position.latitude=latitude;
		m_position.longitude=longitude;
		m_position.r=std::min(m_max_dist, std::max(radius,m_min_dist) );
		m_change_pos=true;
	}

	void move (float offx, float offy)
	{
		m_position.longitude-=offx;
		m_position.latitude-=offy;
		if(m_position.longitude<0)
			m_position.longitude+=float(XMATH_2PI);
		else if(m_position.longitude>=XMATH_2PI)
			m_position.longitude-=float(XMATH_2PI);
		if(m_position.latitude<0.14f)
			m_position.latitude=0.14f;
		else if(m_position.latitude>3.0f)
			m_position.latitude=3.0f;
		m_change_pos=true;
	}

	void zoom (float v)
	{
		m_position.r=std::min(m_max_dist, std::max(m_position.r-v,m_min_dist) );
		m_change_pos=true;
	}

	void set_min_max_distance (float min_dist, float max_dist)
	{
		m_min_dist = min_dist;
		m_max_dist = max_dist;
		m_position.r = std::min(max_dist, std::max(m_position.r,min_dist) );
		m_change_pos = true;
	}

	void update()
	{
		if(m_change_pos)
		{
			m_change_pos = false;
			wyc::xvec3f_t npos;
			wyc::to_cartesian(m_position,npos);
			std::swap(npos.y,npos.z);
			npos.z=-npos.z;
			m_transform.set_position(npos);
			m_transform.set_forward(npos-wyc::xvec3f_t(0,0,0),wyc::xvec3f_t(0,1,0));
		}
		m_transform.update();
	}

	const wyc::xtransform& get_transform() const 
	{
		return m_transform;
	}

	const wyc::xmat4f_t& get_projection() const 
	{
		return m_proj;
	}
};

#endif // __HEADER_WYC_SIMPLE_CAMERA

