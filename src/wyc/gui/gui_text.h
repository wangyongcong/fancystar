#ifndef __HEADER_WYC_FONT
#define __HEADER_WYC_FONT

#include "wyc/render/font.h"
#include "wyc/gui/guiobj.h"

namespace wyc
{

class xgui_text : public xguiobj
{
	USE_RTTI;
	std::string m_font_name;
	xpointer<xfont> m_font;
	xpointer<xfont> m_cur_font; 
	std::wstring m_text;
	std::wstring m_cur_text;
	unsigned m_wait_font;
	unsigned m_wait_glyph;
	typedef std::vector<wyc::xvec3i_t> glyph_sort_buffer_t;
	glyph_sort_buffer_t m_sortbuff;
	bool m_wrap;
public:
	xgui_text ();
	virtual void on_destroy();
	virtual void build_mesh();
	virtual void draw();
	void set_font (const char *font_name);
	xfont* get_font();
	float get_font_size() const;
	void set_text (const wchar_t *text);
	void set_text (const wchar_t *text, size_t pos, size_t count);
	void set_text (const std::wstring &text);
	void set_text (const std::wstring &text, size_t pos, size_t count);
	const std::wstring& get_text() const;
	void set_word_wrap(bool b);
	bool word_wrap() const;
protected:
	static void on_font_ok (unsigned reqid, xrefobj *obj, xresbase *res);
	static void on_glyph_ok (unsigned reqid, xrefobj *obj, xfont *font);
	void load_all_glyphs();
	void rebuild_text_mesh();
	void build_text_mesh();
};

inline xfont* xgui_text::get_font() 
{
	return m_font;
}

inline float xgui_text::get_font_size() const
{
	return m_font?m_font->point_size():0;
}

inline const std::wstring& xgui_text::get_text() const
{
	return m_text;
}

inline void xgui_text::set_word_wrap(bool b)
{
	m_wrap=b;
	rebuild();
}

inline bool xgui_text::word_wrap() const
{
	return m_wrap;
}

}; // namespace wyc

#endif //__HEADER_WYC_FONT
