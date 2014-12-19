#include "fscorepch.h"
#include "scope.h"
#include <cassert>
#include "wyc/render/renderer.h"

using namespace wyc;

REG_RTTI(xscope,xobject);

class xbox_scope : public xscope
{
	USE_RTTI;
public:
	virtual bool create (float radius)
	{
		if(m_vao)
			return true;
		GLuint vao, vbo;
		glGenVertexArrays(1,&vao);
		if(0==vao)
			return false;
		glGenBuffers(1,&vbo);
		if(0==vbo)
		{
			glDeleteVertexArrays(1,&vao);
			return false;
		}
		glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER,vbo);
			glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*8,0,GL_STATIC_DRAW);
			GLfloat *buff=(GLfloat*)glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
			assert(buff);
			buff[0]=-radius;
			buff[1]=-radius;
			buff[2]=radius;
			buff[3]=-radius;
			buff[4]=radius;
			buff[5]=radius;
			buff[6]=-radius;
			buff[7]=radius;
			glUnmapBuffer(GL_ARRAY_BUFFER);
			glVertexAttribPointer(USAGE_POSITION,2,GL_FLOAT,GL_FALSE,0,0);
			glEnableVertexAttribArray(USAGE_POSITION);
		glBindVertexArray(0);

		assert(glGetError()==GL_NO_ERROR);

		m_vao = vao;
		m_vbo = vbo;
		m_vertex_count = 4;
		return true;
	}

};

REG_RTTI(xbox_scope,xscope);

class xcircle_scrope : public xscope
{
	USE_RTTI;
public:
	virtual bool create (float radius)
	{
		if(m_vao)
			return true;
		GLuint vao, vbo;
		glGenVertexArrays(1,&vao);
		if(0==vao)
			return false;
		glGenBuffers(1,&vbo);
		if(0==vbo)
		{
			glDeleteVertexArrays(1,&vao);
			return false;
		}
		glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER,vbo);
			
			float len = float(XMATH_PI * radius * 2);
			unsigned vert_count = unsigned(ceil(len/0.1f));
			float delta = float(XMATH_PI * 2 / vert_count);
			float cos_delta = cos(delta), sin_delta = sin(delta);
			float cos_a = 1.0f, sin_a = 0.0f, ca, sa;
			glBufferData(GL_ARRAY_BUFFER,sizeof(GLfloat)*vert_count*2,0,GL_STATIC_DRAW);
			GLfloat *buff=(GLfloat*)glMapBuffer(GL_ARRAY_BUFFER,GL_WRITE_ONLY);
			assert(buff);
			GLfloat *iter=buff;
			for(unsigned i=0; i<vert_count; ++i)
			{
				*iter++ = radius*cos_a;
				*iter++ = radius*sin_a;

				ca = cos_a*cos_delta - sin_a*sin_delta;
				sa = sin_a*cos_delta + cos_a*sin_delta;
				cos_a = ca;
				sin_a = sa;
			}
			assert(iter==buff+(vert_count*2));
			glUnmapBuffer(GL_ARRAY_BUFFER);
			glVertexAttribPointer(USAGE_POSITION,2,GL_FLOAT,GL_FALSE,0,0);
			glEnableVertexAttribArray(USAGE_POSITION);
		glBindVertexArray(0);
		assert(glGetError()==GL_NO_ERROR);
		
		m_vao = vao;
		m_vbo = vbo;
		m_vertex_count = vert_count;
		return true;
	}

};

REG_RTTI(xcircle_scrope,xscope)
