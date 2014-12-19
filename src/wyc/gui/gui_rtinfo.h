#ifndef __HEADER_WYC_GUI_INFOVIEW
#define __HEADER_WYC_GUI_INFOVIEW

#include "wyc/render/font.h"
#include "wyc/render/text_renderer.h"
#include "wyc/gui/guiobj.h"

namespace wyc
{

class xreq_tracer
{
	std::vector<unsigned> m_reqs;
public:
	void push(unsigned req_id) 
	{
		m_reqs.push_back(req_id);
	}
	void pop(unsigned req_id)
	{
		for(size_t i=0, cnt=m_reqs.size(); i<cnt; ++i) 
		{
			if(m_reqs[i]==req_id) {
				m_reqs[i]=m_reqs.back();
				m_reqs.pop_back();
				return;
			}
		}
	}
	void clear() 
	{
		m_reqs.clear();
	}
	operator bool() const {
		return m_reqs.size()>0;
	}
};

class xgui_rtinfo : public xguiobj
{
	USE_RTTI;
	xpointer<xfont> m_font;
	xmesh2d *m_mesh_info;
	xtext_renderer m_rc_tag;
	xtext_renderer m_rc_info;
	typedef struct {
		uint32_t m_color_tag;
		uint32_t m_color_info;
		std::wstring m_tag;
		std::wstring m_info;
	} entry_t;
	typedef std::vector<entry_t> entry_list_t;
	entry_list_t m_entries;
	size_t m_chars_tag, m_chars_info;
	float m_size_tag, m_size_info;
	uint8_t m_count_per_row;
	bool m_build_tag;
	struct {
		unsigned m_build_tag;
		unsigned m_build_info;
		unsigned m_req_glyph;
		unsigned m_vert_cost;
		unsigned m_vert_used;
		unsigned m_draw;
	} m_debug_stat;
public:
	xgui_rtinfo();
	virtual void on_destroy();
	virtual void build_mesh();
	virtual void draw();
	void set_font(xfont *font);
	void set_entry(const wchar_t *tag, const wchar_t *info);
	void set_entry(const wchar_t *tag, const wchar_t *info, uint32_t tag_color, uint32_t info_color);
	void del_entry(const wchar_t *tag);
	void set_entry_color(const wchar_t *tag, uint32_t tag_color, uint32_t info_color);
	void set_entry_size(float tag_size, float info_size);
	void set_column(unsigned count);
//--------------------------------------------
// debug interface
	void debug_update();
protected:
	static void on_font_ok (xrefobj *obj, xresbase *res);
	void reload_all_glyphs();
	void build_tag_mesh();
	void build_info_mesh();
	static void _draw_text_mesh(void *context, GLuint texture_id, GLushort *index_buffer, size_t index_count);
};

inline void xgui_rtinfo::set_entry_size(float tag_size, float info_size)
{
	m_size_tag=tag_size;
	m_size_info=info_size;
}

inline void xgui_rtinfo::set_column(unsigned count)
{
	m_count_per_row=count;
}

inline void xgui_rtinfo::set_entry(const wchar_t *tag, const wchar_t *info)
{
	set_entry(tag,info,m_color,m_color);
}


}; // namespace wyc

#endif // __HEADER_WYC_GUI_INFOVIEW

