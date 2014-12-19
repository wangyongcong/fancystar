#ifndef __HEADER_WYC_GWEN_RENDERER
#define __HEADER_WYC_GWEN_RENDERER

#include "Gwen/Gwen.h"
#include "Gwen/Renderers/OpenGL.h"

#include "wyc/util/hash.h"
#include "wyc/render/font.h"

namespace wyc
{

class xgwen_renderer : public Gwen::Renderer::OpenGL
{
	typedef Gwen::Renderer::OpenGL BaseClass;
public:
	xgwen_renderer();
	~xgwen_renderer();
	virtual void LoadText( Gwen::Font* pFont, const Gwen::UnicodeString& text );
	virtual void RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text );
	virtual Gwen::Point MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text );
	void set_font(xfont *font);
	xfont* get_font() const;
protected:
	xpointer<xfont> m_font;
	xdict m_fontTextures;
	Gwen::Point m_unknown_size;
	int m_unknown_advance;
	void adjust_pen_pos(Gwen::Point &pos, const Gwen::UnicodeString& text);
};

inline xfont* xgwen_renderer::get_font() const {
	return m_font;
}

}; // namespace wyc

#endif // __HEADER_WYC_GWEN_RENDERER
