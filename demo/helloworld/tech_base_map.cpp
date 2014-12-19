#include "fscorepch.h"

#include "render_tech.h"
#include "shaderlib.h"

using namespace wyc;

class xtech_base_map : public xrender_tech
{
	USE_RTTI;
public:
	virtual void prev_render()
	{
		m_renderer->use_shader(SHADER_BASE_MAP);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
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
		uf=shader->get_uniform("base_map");
		if(-1!=uf)
			glUniform1i(uf,0);
		xtexture *base_map = obj->get_base_map();
		if(base_map && base_map->is_complete())
			glBindTexture(GL_TEXTURE_2D,base_map->handle());
		obj->draw();
	}
};

REG_RTTI(xtech_base_map,xrender_tech)

