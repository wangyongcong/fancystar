#ifndef __HEADER_WYC_SCOPE
#define __HEADER_WYC_SCOPE

#include "wyc/obj/object.h"

class xscope : public wyc::xobject
{
	USE_RTTI;
protected:
	GLuint m_vao, m_vbo;
	unsigned m_vertex_count;
public:
	xscope()
	{
		m_vao=0;
		m_vbo=0;
		m_vertex_count=0;
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
			m_vertex_count=0;
		}
		xobject::on_destroy();
	}
	virtual void draw()
	{
		if(!m_vao)
			return;
		glBindVertexArray(m_vao);
			glDrawArrays(GL_LINE_LOOP,0,m_vertex_count);
		glBindVertexArray(0);
	}
	virtual bool create(float radius) {
		return false;
	};
};


#endif // __HEADER_WYC_SCOPE

