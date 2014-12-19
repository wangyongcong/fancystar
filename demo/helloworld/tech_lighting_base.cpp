#include "fscorepch.h"

#include "tech_lighting_base.h"
#include "simple_light.h"

using namespace wyc;

REG_RTTI(xtech_lighting_base,xrender_tech);

void xtech_lighting_base::prev_render()
{
	if(!m_shader_name || m_lights.empty()) 
		return;
	m_renderer->use_shader(m_shader_name);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_ONE,GL_ONE);

	// transform light position to camera space
	m_light_pos.resize(m_lights.size());
	const xmat4f_t &world2camera = m_camera->get_transform().world2local();
	xvec4f_t pos;
	pos.w = 1;
	for(size_t i=0, count=m_lights.size(); i<count; ++i)
	{
		pos = m_lights[i]->get_transform().position();
		m_light_pos[i]=world2camera*pos;
	}
}
void xtech_lighting_base::draw(xrender_object *obj)
{
	if(!m_shader_name || m_lights.empty()) 
		return;
	xsimple_mesh *mesh = wyc_safecast(xsimple_mesh,obj);
	if(!mesh)
		return;
	const wyc::xshader *shader=m_renderer->get_shader();
	if(!shader)
		return;
	GLint uf;
	const xmat4f_t &world2camera = m_camera->get_transform().world2local();
	const xmat4f_t &local2world = obj->get_transform().local2world();
	xmat4f_t local2camera;
	local2camera.mul(world2camera,local2world);
	xmat4f_t proj2camera;
	proj2camera.mul(m_camera->get_projection(),local2camera);
	uf=shader->get_uniform("local2camera");
	if(-1!=uf)
		glUniformMatrix4fv(uf,1,GL_TRUE,local2camera.data());
	uf=shader->get_uniform("proj2camera");
	if(-1!=uf)
		glUniformMatrix4fv(uf,1,GL_TRUE,proj2camera.data());
	uf=shader->get_uniform("diffuse");
	assert(-1!=uf);
	glUniform3fv(uf,1,mesh->get_diffuse().elem);
	uf=shader->get_uniform("specular");
	assert(-1!=uf);
	glUniform3fv(uf,1,mesh->get_specular().elem);
	uf=shader->get_uniform("smoothness");
	assert(-1!=uf);
	glUniform1f(uf,mesh->get_smoothness());
	GLint light_position=shader->get_uniform("light_position"),
		light_intensity=shader->get_uniform("light_intensity");
	assert(-1!=light_position);
	assert(-1!=light_intensity);

	// custom shader setting
	before_draw(shader,mesh);

	// first pass
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_BLEND);
	xsimple_light *light=m_lights[0];
	glUniform3fv(light_position,1,m_light_pos[0].elem);
	glUniform3fv(light_intensity,1,light->get_intensity().elem);
	obj->draw();

	// disable depth writing
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_EQUAL);
	glEnable(GL_BLEND);
	// one pass per light
	for(size_t i=1, count=m_lights.size(); i<count; ++i)
	{
		light = m_lights[i];
		glUniform3fv(light_position,1,m_light_pos[i].elem);
		glUniform3fv(light_intensity,1,light->get_intensity().elem);
		obj->draw();
	}
}
void xtech_lighting_base::post_render()
{
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
}


