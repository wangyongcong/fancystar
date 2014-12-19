#include "fscorepch.h"

#include "render_tech.h"
#include "shaderlib.h"
#include "simple_mesh.h"

using namespace wyc;

class xtech_normals : public xrender_tech
{
	USE_EVENT_MAP;
	float m_length;
public:
	xtech_normals()
	{
		m_length=0.03f;
	}
	virtual void prev_render()
	{
		m_renderer->use_shader(SHADER_NORMALS);
		const wyc::xshader *shader=m_renderer->get_shader();
		if(!shader)
			return;
		GLint uf=shader->get_uniform("line_length");
		if(-1!=uf)
			glUniform1f(uf,m_length);

		glEnableVertexAttribArray(USAGE_POSITION);
		glEnableVertexAttribArray(USAGE_NORMAL);
	}
	virtual void draw(xrender_object *obj)
	{
		xsimple_mesh *mesh = wyc_safecast(xsimple_mesh,obj);
		if(!mesh)
			return;
		wyc::xvertex_batch *batch = mesh->get_batch();
		if(!batch || !batch->is_complete())
			return;

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
		if(!batch->activate_buffer(USAGE_POSITION))
			return;
		uf=shader->get_uniform("line_color");
		if(batch->activate_buffer(USAGE_NORMAL)) {
			glUniform3f(uf,0,0,1);
			glDrawArrays(GL_POINTS,0,batch->get_vertex_count());
		}

		if(batch->activate_buffer_as(USAGE_TANGENT,USAGE_NORMAL)) {
			glUniform3f(uf,1,0,0);
			glDrawArrays(GL_POINTS,0,batch->get_vertex_count());
		}
	}
	virtual void post_render()
	{
		glDisableVertexAttribArray(USAGE_POSITION);
		glDisableVertexAttribArray(USAGE_NORMAL);

	}

	void set_length(xobjevent *ev)
	{
		if(ev) ev->unpack("f",&m_length);
	}
};

BEGIN_EVENT_MAP(xtech_normals,xrender_tech)
	REG_EVENT(set_length)
END_EVENT_MAP

