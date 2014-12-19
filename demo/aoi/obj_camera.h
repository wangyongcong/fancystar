#ifndef __HEADER_WYC_OBJ_CAMERA
#define __HEADER_WYC_OBJ_CAMERA

#include <cassert>
#include "obj_base.h"

using namespace wyc;

class x3rd_camera : public xbase_object
{
	USE_RTTI;
	xmat4f_t m_proj;
public:
	x3rd_camera ()
	{
	}

	virtual void on_destroy()
	{
	}

	void init_camera(float fov, unsigned view_w, unsigned view_h)
	{
		set_perspective(m_proj,fov,float(view_w)/view_h,1,100000);
	}

	void look_at(const xvec3f_t &from, const xvec3f_t &to)
	{
		m_transform.set_position(from);
		m_transform.set_forward(from-to,xvec3f_t(0,1,0));
	}

	const xmat4f_t& get_projection()
	{
		return m_proj;
	}

};

#endif // __HEADER_WYC_OBJ_CAMERA

