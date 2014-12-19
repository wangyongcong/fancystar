#ifndef __HEADER_WYC_TILEBUFFER
#define __HEADER_WYC_TILEBUFFER

#include "wyc/render/vertexbatch.h"

namespace wyc
{

struct xvertex2d
{
	xvec2f_t m_pos;
	xvec2f_t m_texcoord;
	uint32_t m_color;
};

class xtile_buffer;

class xmesh2d : public xrefobj
{
	friend class xtile_buffer;
	union {
		xtile_buffer *m_parent;
		xmesh2d *m_free_link;
	};
	unsigned m_base;
	unsigned m_count;
	xmesh2d(xtile_buffer *parent, unsigned base, unsigned count) {
		m_parent=parent;
		m_base=GLushort(base);
		m_count=count;
	}
public:
	virtual void delthis();
	const xvertex2d& operator [] (size_t idx) const;
	xvertex2d& operator [] (size_t idx);
	const xvertex2d& get_vertex(size_t idx) const;
	xvertex2d& get_vertex(size_t idx);
	unsigned vertex_count() const;
	void commit_vertex();
	void draw(unsigned short *index, unsigned count, unsigned tex_id, unsigned shader);
};

class xtile_buffer : public xrefobj
{
public:
	xtile_buffer();
	virtual void delthis();
	xmesh2d* alloc_mesh(unsigned vertex_count);
	void free_mesh(xmesh2d *pmesh);
	bool alloc_mesh(xmesh2d** mesh_array, unsigned mesh_count, unsigned vertices_per_mesh);
	void free_mesh(xmesh2d** mesh_array, unsigned mesh_count);
	void mark_section(unsigned base, unsigned size);
	void clear_draw_buffer();
	void draw_mesh(unsigned base, unsigned count, unsigned short *index, unsigned index_count, GLuint tex, unsigned shader);
	void render(xrenderer *renderer);
	void reserve(unsigned vertex_count, unsigned index_count);
	void set_read_only(bool b);
	bool read_only() const;
	static unsigned max_vertex_count();
private:
	void r_init_buffers();
	void r_update_buffers();

	enum { CACHE_LEVEL = 9 };
	static const unsigned ms_cache_size[CACHE_LEVEL];
	xmesh2d *m_free_mesh[CACHE_LEVEL];
	std::vector<xmesh2d*> m_mesh_record;
	std::vector<xvertex2d> m_vertex_buff;
	std::vector<GLushort> m_idx_buff;
	unsigned m_cap_vertex, m_cap_index;
	typedef std::pair<unsigned,unsigned> section_t;
	struct section_less {
		inline bool operator () (const section_t &l, const section_t &r) const {
			return l.first<r.first;
		}
	};
	typedef std::set<section_t,section_less> section_list_t;
	section_list_t m_commit_section; 
	struct xdraw_section
	{
		GLuint m_tex;
		unsigned m_shader;
		unsigned m_begin;
		unsigned m_count;
	};
	typedef std::vector<xdraw_section> draw_buffer_t;
	draw_buffer_t m_draw_buff;
	GLuint m_vao;
	union {
		GLuint m_vbuffs[2];
		struct {
			GLuint m_vbo;
			GLuint m_ibo;
		};
	};
	bool m_read_only;
	bool m_commit_index;
	friend class xmesh2d;
};

inline void xmesh2d::delthis() {
	if(m_parent)
		m_parent->free_mesh(this);
	else xrefobj::delthis();
}

inline const xvertex2d& xmesh2d::operator [] (size_t idx) const {
	return m_parent->m_vertex_buff[m_base+idx];
}

inline xvertex2d& xmesh2d::operator [] (size_t idx) {
	return m_parent->m_vertex_buff[m_base+idx];
}

inline const xvertex2d& xmesh2d::get_vertex(size_t idx) const {
	return m_parent->m_vertex_buff[m_base+idx];
}

inline xvertex2d& xmesh2d::get_vertex(size_t idx) {
	return m_parent->m_vertex_buff[m_base+idx];
}

inline void xmesh2d::commit_vertex() {
	m_parent->mark_section(m_base,m_count);
}

inline unsigned xmesh2d::vertex_count() const {
	return m_count;
}

inline void xmesh2d::draw(unsigned short *index, unsigned index_count, unsigned tex_id, unsigned shader) {
	m_parent->draw_mesh(m_base, m_count, index, index_count, tex_id, shader);
}

inline unsigned xtile_buffer::max_vertex_count()
{
	return ms_cache_size[CACHE_LEVEL-1];
}

inline void xtile_buffer::reserve(unsigned vertex_count, unsigned index_count)
{
	m_vertex_buff.reserve(vertex_count);
	m_idx_buff.reserve(index_count);
}

inline void xtile_buffer::set_read_only(bool b)
{
	m_read_only=b;
}

inline bool xtile_buffer::read_only() const
{
	return m_read_only;
}

}; // namespace wyc

#endif // __HEADER_WYC_TILEBUFFER
