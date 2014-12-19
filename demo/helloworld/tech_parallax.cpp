#include "fscorepch.h"
#include "tech_parallax.h"

using namespace wyc;

xtech_parallax::xtech_parallax() : xtech_lighting_base()
{
	m_shader_name = SHADER_PARALLAX;
}

void xtech_parallax::before_draw(const xshader *shader, xsimple_mesh *mesh)
{
	GLint uf=shader->get_uniform("base_map");
	assert(-1!=uf);
	glUniform1i(uf,0);
	uf=shader->get_uniform("normal_map");
	assert(-1!=uf);
	glUniform1i(uf,1);
	uf=shader->get_uniform("height_map");
	assert(-1!=uf);
	glUniform1i(uf,2);
	xtexture *tex;
	tex=mesh->get_height_map();
	if(tex && tex->is_complete()) {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D,tex->handle());
	}	
	tex=mesh->get_normal_map();
	if(tex && tex->is_complete()) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D,tex->handle());
	}
	tex=mesh->get_base_map();
	if(tex && tex->is_complete()) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,tex->handle());
	}
}

void xtech_parallax::post_render()
{
	xtech_lighting_base::post_render();
	glActiveTexture(GL_TEXTURE0);
}

REG_RTTI(xtech_parallax,xtech_lighting_base);
