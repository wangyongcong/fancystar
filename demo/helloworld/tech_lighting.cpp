#include "fscorepch.h"

#include "tech_lighting_base.h"

using namespace wyc;

class xtech_lighting : public xtech_lighting_base
{
	USE_RTTI;
public:
	xtech_lighting() : xtech_lighting_base()
	{
		m_shader_name = SHADER_LIGHTING;
	}
	virtual void before_draw(const xshader *shader, xsimple_mesh *mesh)
	{
		GLint uf=shader->get_uniform("base_map");
		assert(-1!=uf);
		glUniform1i(uf,0);
		xtexture *base_map=mesh->get_base_map();
		if(base_map && base_map->is_complete())
			glBindTexture(GL_TEXTURE_2D,base_map->handle());
	}
};

REG_RTTI(xtech_lighting,xtech_lighting_base);


