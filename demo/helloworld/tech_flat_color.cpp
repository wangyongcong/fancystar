#include "fscorepch.h"

#include "tech_lighting_base.h"

using namespace wyc;

class xtech_flat_color : public xtech_lighting_base
{
	USE_EVENT_MAP;
	xvec3f_t m_color;
public:
	xtech_flat_color() : xtech_lighting_base()
	{
		m_shader_name = SHADER_FLAT_COLOR;
		m_color.set(1,1,1);
	}
	virtual void before_draw(const xshader *shader, xsimple_mesh *mesh)
	{
		GLint uf=shader->get_uniform("color");
		assert(-1!=uf);
		glUniform3f(uf, m_color.x, m_color.y, m_color.z);
	}
	void set_color(xobjevent *ev)
	{
		if(ev) ev->unpack("fff",&m_color.x,&m_color.y,&m_color.z);
	}
};

BEGIN_EVENT_MAP(xtech_flat_color,xrender_tech)
	// register events
	REG_EVENT(set_color)
END_EVENT_MAP

