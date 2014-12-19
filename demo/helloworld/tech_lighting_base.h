#ifndef __HEADER_WYC_TECH_LIGHTING_BASE
#define __HEADER_WYC_TECH_LIGHTING_BASE

#include "render_tech.h"
#include "simple_mesh.h"
#include "shaderlib.h"

namespace wyc {

class xtech_lighting_base : public xrender_tech
{
	USE_RTTI;
protected:
	unsigned int m_shader_name;
public:
	xtech_lighting_base()
	{
		m_shader_name=0;
	}
	virtual void prev_render();
	virtual void draw(xrender_object *obj);
	virtual void post_render();
	virtual void before_draw(const xshader *shader, xsimple_mesh *mesh)
	{
	}
};

} // end of namespace wyc

#endif // __HEADER_WYC_TECH_LIGHTING_BASE