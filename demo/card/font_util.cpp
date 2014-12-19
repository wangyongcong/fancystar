#include "fscorepch.h"
#include "font_util.h"

#include "wyc/game/game.h"

namespace wyc
{

void async_load_font(void *param);
void async_load_font_callback(void *param);
void async_load_glyphs(void *param);
void async_load_glyphs_callback(void *param);

struct xtask_load_font : public xrefobj
{
	xpointer<xfont> font;
	std::string font_name;
};

struct xtask_load_glyphs : public xrefobj
{
	xpointer<xfont> font;
	xfont::glyph_t **glyphs;
	size_t size;
};

xfont* get_font(const char *font_name)
{
	xgame &game=xgame::singleton();
	xfont *font = (xfont*)game.resource_server().request(xfont::get_class()->id,font_name);
	if(!font->is_complete() && !font->is_loading()) {
		xtask_load_font *task = wycnew xtask_load_font;
		task->font=font;
		task->font_name=font_name;
		game.send_request(&async_load_font,task,&async_load_font_callback);
	}
	return font;
}

class xglyph_buffer 
{
	xfont::glyph_t **m_buffer;
	size_t m_cap, m_size;
public:
	xglyph_buffer(xfont::glyph_t **buffer=0, size_t size=0, size_t cap=0) : m_buffer(buffer), m_cap(cap), m_size(size)
	{
	}
	~xglyph_buffer()
	{
		if(m_buffer)
			delete [] m_buffer;
	}
	void detach(xfont::glyph_t** &buff, size_t &size)
	{
		buff=m_buffer;
		size=m_size;
		m_buffer=0;
		m_size=0;
	}
	void append(xfont::glyph_t *glyph)
	{
		if(m_size+1>m_cap)
		{
			m_cap = m_cap*2+1;
			xfont::glyph_t **buffer = new xfont::glyph_t*[m_cap];
			if(m_buffer) {
				memcpy(buffer,m_buffer,sizeof(xfont::glyph_t*)*m_size);
				delete [] m_buffer;
			}
			m_buffer=buffer;
		}
		glyph->flag=LOADING;
		m_buffer[m_size]=glyph;
		m_size+=1;
	}
	void complete()
	{
		for(size_t i=0; i<m_size; ++i) 
		{
			m_buffer[i]->flag=COMPLETE;
		}
	}
};

inline bool is_loading(const xfont::glyph_t *glyph)
{
	return UNDEFINED==glyph->flag;
}

inline bool is_complete(const xfont::glyph_t *glyph)
{
	return LOADING==glyph->flag;
}

unsigned load_glyphs(xfont *font, const wchar_t *chars)
{
	unsigned send=0;
	const wchar_t *iter;
	xfont::glyph_t *glyph;
	xglyph_buffer gb;
	for(iter=chars; *iter; ++iter)
	{
		glyph=font->get_glyph(*iter);
		if(is_complete(glyph))
			continue;
		if(!is_loading(glyph)) {
			gb.append(glyph);
			++send;
		}
	}
	if(send)
	{
		xtask_load_glyphs *task = wycnew xtask_load_glyphs;
		task->font=font;
		gb.detach(task->glyphs,task->size);
		xgame &game=xgame::singleton();
		game.send_request(&async_load_glyphs,task,&async_load_glyphs_callback);
	}
	return send;
}

void async_load_font(void *param)
{
	xtask_load_font *task = (xtask_load_font*)param;
	if(!task->font->load(task->font_name.c_str()))
	{
		wyc_warn("Failed to load font: [%s]",task->font_name.c_str());
	}
}

void async_load_font_callback(void *param)
{
	xtask_load_font *task = (xtask_load_font*)param;
	if(task->font->get_ready())
		task->font->set_complete();
}

void async_load_glyphs(void *param)
{
	xtask_load_glyphs *task = (xtask_load_glyphs*)param;
}

void async_load_glyphs_callback(void *param)
{
	xtask_load_glyphs *task = (xtask_load_glyphs*)param;
	xglyph_buffer gb(task->glyphs,task->size);
	gb.complete();
}


}; // namespace wyc
