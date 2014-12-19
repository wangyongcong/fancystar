#ifndef __HEADER_WYC_SIMPLE_LIGHT
#define __HEADER_WYC_SIMPLE_LIGHT

#include "wyc/math/vecmath.h"
#include "wyc/math/transform.h"
#include "wyc/obj/object.h"

#include "render_object.h"

class xsimple_light : public xrender_object
{
	USE_RTTI;
	wyc::xvec3f_t m_axis;
	float m_speed;
	wyc::xvec3f_t m_intensity;
public:
	xsimple_light()
	{
		m_speed=0;
		m_intensity.set(1,1,1);
	}

	virtual void update_transform(float t)
	{
		if(m_speed!=0) {
			wyc::xmat3f_t rot;
			wyc::matrix_rotate3d(rot,m_axis,m_speed*t);
			wyc::xvec3f_t pos = m_transform.position()*rot;
			m_transform.set_position(pos);
		}
	}

	void set_rotation (const wyc::xvec3f_t &axis, float radius, float speed)
	{
		wyc::xvec3f_t look, right;
		float v1 = axis.dot(wyc::xvec3f_t(1,0,0)), 
			v2 = axis.dot(wyc::xvec3f_t(0,1,0));
		if(v1>v2)
			look.set(0,1,0);
		else
			look.set(1,0,0);
		right.cross(axis,look);
		right.normalize();
		m_transform.set_position(right*radius);
		m_axis=axis;
		m_axis.normalize();
		m_speed=speed;
		update_transform(wyc::random());
	}

	void set_intensity(float i)
	{
		m_intensity.set(i,i,i);
	}
	void set_intensity(float r, float g, float b)
	{
		m_intensity.set(r,g,b);
	}
	void set_intensity(const wyc::xvec3f_t &i)
	{
		m_intensity=i;
	}
	const wyc::xvec3f_t& get_intensity() const
	{
		return m_intensity;
	}
};

#endif // __HEADER_WYC_SIMPLE_LIGHT

