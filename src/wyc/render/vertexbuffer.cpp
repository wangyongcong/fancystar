#include "fscorepch.h"
#include "wyc/render/vertexbuffer.h"

namespace wyc
{

REG_RTTI(xvertex_buffer,xresbase);

xvertex_buffer::xvertex_buffer()
{
	m_vbo=0;
}

bool xvertex_buffer::load(const char *name)
{
	if(m_vbo) 
		unload();
	glGenBuffers(1,&m_vbo);
	// TODO: parse script or load from file
	return true;
}

void xvertex_buffer::unload()
{
	if(m_vbo) {
		glDeleteBuffers(1,&m_vbo);
		m_vbo=0;
	}
}

}; // namespace wyc

