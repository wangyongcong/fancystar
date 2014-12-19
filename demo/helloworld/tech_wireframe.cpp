#include "fscorepch.h"

#include "render_tech.h"
#include "shaderlib.h"

using namespace wyc;

class xtech_wireframe : public xrender_tech
{
	USE_EVENT_MAP;
	xvec3f_t m_color;
public:
	xtech_wireframe()
	{
		m_color.set(0,0,0);
	}
	virtual void prev_render()
	{
		m_renderer->use_shader(SHADER_FLAT_COLOR);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-0.4f,-1);
		glPolygonMode(GL_FRONT,GL_LINE);
	}
	virtual void draw(xrender_object *obj)
	{
		const wyc::xshader *shader=m_renderer->get_shader();
		const xmat4f_t &world2camera = m_camera->get_transform().world2local();
		const xmat4f_t &local2world = obj->get_transform().local2world();
		xmat4f_t local2camera;
		local2camera.mul(world2camera,local2world);
		xmat4f_t proj2camera;
		proj2camera.mul(m_camera->get_projection(),local2camera);
		GLint uf;
		uf=shader->get_uniform("proj2camera");
		if(-1!=uf)
			glUniformMatrix4fv(uf,1,GL_TRUE,proj2camera.data());
		uf=shader->get_uniform("color");
		if(-1!=uf)
			glUniform3f(uf,m_color.x,m_color.y,m_color.z);
		obj->draw();
	}
	virtual void post_render()
	{
		glDisable(GL_POLYGON_OFFSET_LINE);
		glPolygonMode(GL_FRONT,GL_FILL);
	}

	void set_color(xobjevent *ev)
	{
		if(ev) ev->unpack("fff",&m_color.x,&m_color.y,&m_color.z);
	}
};

BEGIN_EVENT_MAP(xtech_wireframe,xrender_tech)
	REG_EVENT(set_color)
END_EVENT_MAP

