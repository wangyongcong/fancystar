#include "fscorepch.h"

#include "render_tech.h"
#include "shaderlib.h"

using namespace wyc;

class xtech_billboard : public xrender_tech
{
	USE_RTTI;
	GLuint m_point_buffer;
	size_t m_buffer_size;
	std::vector<xrender_object*> m_objs;
public:
	xtech_billboard() : xrender_tech()
	{
		m_point_buffer=0;
		m_buffer_size=0;
	}
	virtual void on_destroy()
	{
		if(m_point_buffer)
			glDeleteBuffers(1,&m_point_buffer);
	}

	virtual void prev_render()
	{
		m_renderer->use_shader(SHADER_POINT_SPRITE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		if(!glIsEnabled(GL_POINT_SPRITE)) {
			glEnable(GL_POINT_SPRITE);
			glEnable(GL_PROGRAM_POINT_SIZE);
			glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN,GL_LOWER_LEFT);
		}
	}
	virtual void draw(xrender_object *obj)
	{
		xtexture *tex = obj->get_base_map();
		if(!tex) return;
		m_objs.push_back(obj);
	}
	virtual void post_render()
	{
		if(!m_objs.size())
			return;
		_resize_point_buffer(m_objs.size());
		_update_vertices();
		glVertexAttribPointer(USAGE_POSITION,3,GL_FLOAT,GL_FALSE,0,0);
		glEnableVertexAttribArray(USAGE_POSITION);
		const wyc::xshader *shader=m_renderer->get_shader();
		const xmat4f_t &world2camera = m_camera->get_transform().world2local();
		xmat4f_t proj2camera;
		proj2camera.mul(m_camera->get_projection(),world2camera);
		GLint uf;
		uf=shader->get_uniform("proj2camera");
		if(-1!=uf)
			glUniformMatrix4fv(uf,1,GL_TRUE,proj2camera.data());
		uf=shader->get_uniform("base_map");
		if(-1!=uf)
			glUniform1i(uf,0);
		xtexture *base_map=0;
		size_t vert_offset=0, vert_count=0;
		for(size_t i=0, count=m_objs.size(); i<count; ++i)
		{
			if(base_map!=m_objs[i]->get_base_map())
			{
				if(vert_count>0)
				{
					glBindTexture(GL_TEXTURE_2D,base_map->handle());
					glDrawArrays(GL_POINTS,vert_offset,vert_count);
					vert_count=0;
				}
				base_map = m_objs[i]->get_base_map();
				if(!base_map || !base_map->is_complete())
				{
					for(i+=1; i<count && m_objs[i]->get_base_map()==base_map; ++i);
					if(i>=count)
						break;
					base_map = m_objs[i]->get_base_map();
				}
				vert_offset= sizeof(GLfloat)*3*i;
			}
			vert_count += 1;
		}
		if(vert_count>0)
		{
			glBindTexture(GL_TEXTURE_2D,base_map->handle());
			glDrawArrays(GL_POINTS,vert_offset,vert_count);
		}

		m_objs.clear();

		glDepthMask(GL_TRUE);
	}


protected:
	void _resize_point_buffer(size_t obj_count)
	{
		size_t size_needed = sizeof(GLfloat)*3*obj_count;
		if(size_needed<=m_buffer_size)
			return;
		if(m_point_buffer) {
			glDeleteBuffers(1,&m_point_buffer);
			m_point_buffer=0;
		}
		glGenBuffers(1,&m_point_buffer);
		glBindBuffer(GL_ARRAY_BUFFER,m_point_buffer);
		m_buffer_size = (size_needed + 255) & ~255;
		glBufferData(GL_ARRAY_BUFFER,m_buffer_size,0,GL_DYNAMIC_DRAW);
	}

	void _update_vertices()
	{
		glBindBuffer(GL_ARRAY_BUFFER,m_point_buffer);
		size_t length = sizeof(GLfloat)*3*m_objs.size();
		GLfloat *buff = (GLfloat*)glMapBufferRange(GL_ARRAY_BUFFER,0,length,GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_RANGE_BIT);
		if(!buff) 
			return;
		GLfloat *iter=buff;
		xrender_object *obj;
		xtexture *base_map, *prev_map=m_objs[0]->get_base_map();
		size_t search_pos = 0;
		for(size_t i=0, count=m_objs.size(); i<count; ++i)
		{
			obj=m_objs[i];
			base_map=obj->get_base_map();
			if(base_map!=prev_map) 
			{
				size_t j = std::max(search_pos,i+1);
				for(; j<count; ++j)
				{
					if(m_objs[j]->get_base_map()==prev_map)
					{
						search_pos = j+1;
						std::swap(m_objs[i],m_objs[j]);
						obj=m_objs[i];
						break;
					}
				}
				if(j>=count) // not found
				{
					prev_map = obj->get_base_map();
					search_pos = i+1;
				}
			}
			const xvec3f_t &pos= obj->get_transform().position();
			*iter++=pos.x;
			*iter++=pos.y;
			*iter++=pos.z;
		}
		assert(iter==buff+m_objs.size()*3);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

};

REG_RTTI(xtech_billboard,xrender_tech)

