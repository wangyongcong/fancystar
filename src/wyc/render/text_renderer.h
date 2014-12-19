#ifndef __HEADER_WYC_TEXT_RENDERER
#define __HEADER_WYC_TEXT_RENDERER

#include "wyc/render/tilebuffer.h"
#include "wyc/render/font.h"

namespace wyc
{

class xtext_renderer 
{
public:
	xtext_renderer();
	~xtext_renderer();
	void attach_buffer(void *vert_buff, size_t size);
	void detach_buffer();
	void clear_buffer();
	void draw_text(float &pen_x, float &pen_y, const wchar_t *text, xfont *font, uint32_t color, float scale, const xrectf_t *bounding_box=0, int style=0);
	typedef void (*render_func_t) (void *context, GLuint texture_id, GLushort *index_buffer, size_t index_count);
	void render();
	size_t get_buffer_size() const;
	void set_custom_renderer(render_func_t rc);
	void set_render_context(void *context);
	static size_t calc_buffer_size(size_t char_count);
private:
	xvertex2d *m_vbuffer;
	xvertex2d *m_vert_end;
	size_t m_vbuffer_size;
	size_t m_vert_count;
	typedef std::vector<wyc::xvec3i_t> draw_buffer_t;
	draw_buffer_t m_draw_buffer;
	render_func_t m_custom_renderer;
	void *m_custom_context;
	bool m_need_sort;
	void sort_mesh();
};

inline size_t xtext_renderer::get_buffer_size() const
{
	return m_vert_count*sizeof(xvertex2d);
}

inline size_t xtext_renderer::calc_buffer_size(size_t char_count) 
{
	return char_count * sizeof(xvertex2d) * 4;
}

inline void xtext_renderer::set_custom_renderer(render_func_t rc)
{
	m_custom_renderer = rc;
}

inline void xtext_renderer::set_render_context(void *context)
{
	m_custom_context = context;
}

}; // namespace wyc

#endif // __HEADER_WYC_TEXT_RENDERER
