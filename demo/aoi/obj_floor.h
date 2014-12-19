#ifndef __HEADER_WYC_OBJ_FLOOR
#define __HEADER_WYC_OBJ_FLOOR

#include <cassert>
#include "obj_base.h"
#include "shaderlib.h"

using namespace wyc;

class xfloor : public xbase_object
{
	USE_RTTI;
	GLuint m_vao, m_vbo;
	unsigned m_vert_count;
	xvec3f_t m_color;
public:
	xfloor()
	{
		m_vao=0;
		m_vbo=0;
		m_vert_count=0;
		m_color.set(1,1,1);
	}

	virtual void on_destroy()
	{
		if(m_vao) 
		{
			glDeleteVertexArrays(1,&m_vao);
			m_vao=0;
			assert(m_vbo);
			glDeleteBuffers(1,&m_vbo);
			m_vbo=0;
		}
		xbase_object::on_destroy();
	}

	bool create (unsigned size, float grid_size=1)
	{
		unsigned line_count = size+1;
		unsigned vert_count = 4*line_count;
		unsigned count = vert_count*2;
		GLfloat *vertices = new GLfloat[count];
		GLfloat *iter = vertices;
		GLfloat upper = size*grid_size*0.5f, lower = -upper;
		GLfloat x, y;
		// fill rows
		y=upper;
		for(unsigned i=0; i<line_count; ++i)
		{
			*iter++=lower;
			*iter++=y;
			*iter++=upper;
			*iter++=y;
			y-=grid_size;
		}
		// fill cols
		x=lower;
		for(unsigned i=0; i<line_count; ++i)
		{
			*iter++=x;
			*iter++=upper;
			*iter++=x;
			*iter++=lower;
			x+=grid_size;
		}
		assert(iter==vertices+count);
		bool is_ok = build_mesh(vertices,count);
		delete [] vertices;

		if(is_ok)
		{
			m_vert_count=vert_count;
		}
		return is_ok;
	}

	bool build_mesh(GLfloat *vertices, unsigned size)
	{
		// OGL
		GLuint vao, vbo;
		glGenVertexArrays(1,&vao);
		if(!vao)
			return false;
		glGenBuffers(1,&vbo);
		if(!vbo)
		{
			glDeleteVertexArrays(1,&vao);
			return false;
		}
		glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER,vbo);
			glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*size,vertices,GL_STATIC_DRAW);
			glVertexAttribPointer(USAGE_POSITION,2,GL_FLOAT,GL_FALSE,0,0);
			glEnableVertexAttribArray(USAGE_POSITION);
		glBindVertexArray(0);

		m_vao=vao;
		m_vbo=vbo;
		return true;
	}

	virtual void draw (const xshader *sh)
	{
		if(!m_vao) 
			return;
		GLint uf;
		uf=sh->get_uniform("mat_model");
		if(-1==uf)
			return;
		glUniformMatrix4fv(uf,1,GL_TRUE,m_transform.local2world().data());
		uf=sh->get_uniform("color");
		if(-1==uf)
			return;
		glUniform3f(uf,m_color.x,m_color.y,m_color.z);
		glBindVertexArray(m_vao);
			glDrawArrays(GL_LINES,0,m_vert_count);
		glBindVertexArray(0);
	}

	void set_color(const xvec3f_t &c)
	{
		m_color=c;
	}

	const xvec3f_t& get_color() const
	{
		return m_color;
	}
};

#endif // __HEADER_WYC_OBJ_FLOOR
