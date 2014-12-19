#ifndef __HEADER_WYC_FONT_SVR
#define __HEADER_WYC_FONT_SVR

#include "wyc/util/fjson.h"
#include "wyc/obj/object.h"
#include "wyc/obj/resbase.h"
#include "wyc/thread/spsc_ring.h"

#include "freetype-gl/texture-font.h"
#include "freetype-gl/glyph-dict.h"

namespace wyc
{

class xfont : public xresbase
{
	USE_RTTI;
public:
//	typedef texture_glyph_t glyph_t;
	typedef void (*glyph_callback_t) (uintptr_t reqid, xrefobj *subscriber, xfont *font);
	xfont();
	virtual bool load(const char *font_name);
	virtual void unload();
	virtual bool async_load(const char *font_name);
	texture_glyph_t* get_glyph(wchar_t charcode);
	unsigned async_load_glyphs(const wchar_t *chars, size_t len=0, int style=0);
	void upload_texture();
	void reserve_dict(size_t size);	
	const char* name() const;
	float point_size() const;
	float font_height() const;
	float ascender() const;
	float descender() const;
	float kerning(wchar_t left_char, wchar_t right_char);
private:
	struct xfont_task : public xrefobj
	{
		xfont_task *m_next;
		xpointer<xfont> m_font;
		texture_glyph_t **m_buffer;
		size_t m_size;
		int m_style;
		virtual void delthis() {
			m_font=0;
			xrefobj::delthis();
		}
	};
	xfont_task* new_task();
	void del_task(xfont_task*);

	texture_font_t *m_internal_font;
	glyph_dict_t *m_dict;
	xfont_task *m_task_pool;
	std::vector<void*> m_glyph_buffers;
	unsigned m_bucket_used;
	xdict m_kerning;

	texture_glyph_t* new_glyph_buffer(const wchar_t *chars, size_t sz, size_t &pos, size_t &count);
	texture_glyph_t* new_glyph(wchar_t ch);
	static void _async_render_glyph(xfont_task *task);
	static void _async_render_glyph_callback(xfont_task *task);
};

extern unsigned generate_font_id(const char*, float);

inline const char* xfont::name() const
{
	return m_internal_font->filename;
}

inline float xfont::point_size() const
{
	return m_internal_font->size;
}

inline float xfont::font_height() const
{
	return m_internal_font->height;
}

inline float xfont::ascender() const 
{
	return m_internal_font->ascender;
}

inline float xfont::descender() const
{
	return m_internal_font->descender;
}

inline texture_glyph_t* xfont::get_glyph(wchar_t charcode)
{
	assert(m_dict);
	return glyph_dict_get(m_dict,charcode);
}

}; // namespace wyc

#endif // __HEADER_WYC_FONT_SVR
