#ifndef __HEADER_WYC_VERTEXBATCH
#define __HEADER_WYC_VERTEXBATCH

#include "wyc/obj/resbase.h"
#include "wyc/render/renderer.h"

namespace wyc
{

class xvertex_buffer : public xrefobj
{
	GLuint m_vbo;
	GLenum m_hint;
	uint8_t *m_buffer;
	size_t m_size;
public:
	xvertex_buffer()
	{
		m_vbo=0;
		m_hint=GL_STATIC_DRAW;
		m_buffer=0;
		m_size=0;
	}
	virtual void delthis()
	{
		if(m_vbo) {
			glDeleteBuffers(1,&m_vbo);
			m_vbo=0;
		}
		release_buffer();
		xrefobj::delthis();
	}
	void alloc_buffer(size_t sz, unsigned hint=GL_STATIC_DRAW) 
	{
		if(m_buffer) 
			delete [] m_buffer;
		m_buffer=new uint8_t[sz];
		m_size=sz;
		m_hint=hint;
	}
	void release_buffer() {
		if(m_buffer) {
			delete [] m_buffer;
			m_buffer=0;
			m_size=0;
		}
	}
	inline uint8_t* get_buffer() 
	{
		return m_buffer;
	}
	inline size_t size() 
	{
		return m_size;
	}
	inline GLenum usage_hint() const
	{
		return m_hint;
	}
	inline GLuint handle() const 
	{
		return m_vbo;
	}
	inline void set_handle(GLuint vbo) {
		if(m_vbo) 
			glDeleteBuffers(1,&m_vbo);
		m_vbo=vbo;
	}
};

class xvertex_batch : public xresbase
{
	USE_RTTI;
	GLuint m_vao;
	GLenum m_mode;
	GLenum m_index_type;
	unsigned m_vertex_count;
	unsigned m_triangel_count;
	unsigned m_prim_count;
	unsigned m_offset;
	struct xbuffer_attribute
	{
		xbuffer_attribute *m_next;
		SHADER_USAGE m_usage;
		unsigned m_stride;
		unsigned m_offset;
		unsigned m_type;
		uint8_t m_component;
		bool m_normalize;
	};
	typedef std::pair<xvertex_buffer*,xbuffer_attribute*> xbuffer_entry;
	std::vector<xbuffer_entry> m_buffers;
	xpointer<xvertex_buffer> m_index_buff;
public:
	xvertex_batch();
	virtual bool load(const char *res_name);
	virtual bool async_load(const char *res_name);
	virtual void unload();
	virtual void on_async_complete();
	GLuint handle() const;
	void set_mode(unsigned mode, unsigned count, unsigned offset=0);
	void set_index(xvertex_buffer *pbuff, unsigned index_type=GL_UNSIGNED_INT);
	void add_buffer(xvertex_buffer *pbuff, SHADER_USAGE index, unsigned component, unsigned data_type, \
		bool normalize=false, unsigned stride=0, unsigned offset=0);
	void render();
	unsigned get_vertex_count() const;
	bool activate_buffer (SHADER_USAGE name);
	bool activate_buffer_as (SHADER_USAGE name, SHADER_USAGE used_as);
private:
	static void calculate_tangents(unsigned vertexCount, const xvec3f_t *vertex, const xvec3f_t *normal, 
		const xvec2f_t *texcoord, unsigned indexCount, const unsigned short *index, xvec4f_t *tangent);
	bool r_build_vertex_array();
	bool r_build_vertex_buffers();
	void r_render_buffers();
	void generate_mesh_cube(float size);
	void generate_mesh_cylinder(float radius, unsigned fraction, float height);
	void generate_mesh_sphere(float radius, unsigned fraction);
	void generate_mesh_torus(float main_radius, unsigned main_fraction, float pipe_radius, unsigned pipe_fraction);
};

//-------------------------------------------------------------------------------------

inline GLuint xvertex_batch::handle() const {
	return m_vao;
}

inline void xvertex_batch::set_mode(unsigned mode, unsigned count, unsigned offset) 
{
	m_mode=mode;
	m_prim_count=count;
	m_offset=offset;
}

inline void xvertex_batch::set_index(xvertex_buffer *pbuff, unsigned index_type) 
{
	assert(pbuff);
	m_index_buff=pbuff;
	m_index_type=index_type;
}

inline unsigned xvertex_batch::get_vertex_count() const
{
	return m_vertex_count;
}

inline bool xvertex_batch::activate_buffer (SHADER_USAGE name)
{
	return activate_buffer_as(name,name);
}

}; // namespace wyc

#endif // __HEADER_WYC_VERTEXBATCH

