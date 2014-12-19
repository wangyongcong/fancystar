#ifndef __HEADER_WYC_VERTEXBUFFER
#define __HEADER_WYC_VERTEXBUFFER

#include "wyc/render/renderer.h"
#include "wyc/obj/resbase.h"

namespace wyc
{

class xvertex_buffer : public xresbase
{
	USE_RTTI;
	GLuint m_vbo;
public:
	xvertex_buffer();
	virtual bool load(const char *name);
	virtual void unload();
	inline GLuint handle() const {
		return m_vbo;
	}
	inline void set_handle(unsigned vbo) {
		m_vbo=vbo;
	}
};

}; // namespace wyc

#endif // __HEADER_WYC_VERTEXBUFFER

