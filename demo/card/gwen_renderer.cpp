#include "fscorepch.h"
#include <math.h>
#include "Gwen/Utility.h"
#include "Gwen/Font.h"
#include "Gwen/Texture.h"
#include "wyc/game/glb_game.h"
#include "gwen_renderer.h"

namespace wyc
{

xgwen_renderer::xgwen_renderer()
{
	m_unknown_size.x = 0;
	m_unknown_size.y = 0;
	m_unknown_advance= 0;
}

xgwen_renderer::~xgwen_renderer()
{
	m_font = 0;
	Gwen::Texture *texture;
	for(xdict::iterator iter=m_fontTextures.begin(), end=m_fontTextures.end(); iter!=end; ++iter)
	{
		texture = (Gwen::Texture*)iter->second;
		delete (GLuint*)texture->data;
		delete texture;
	}
}

void xgwen_renderer::set_font(xfont *font) {
	assert(font);
	assert(font->is_complete());
	m_font = font;
	texture_glyph_t *glyph = m_font->get_glyph(L'?');
	if(glyph) {
		m_unknown_size.x = int(glyph->width+0.5f);
		m_unknown_size.y = int(glyph->height+0.5f);
		m_unknown_advance= int(glyph->advance_x+0.5f);
	}
	else {
		m_unknown_size.x = int(m_font->font_height()*0.8f+0.5f);
		m_unknown_size.y = int(m_font->font_height()+0.5f);
		m_unknown_advance= m_unknown_size.x+2;
	}
}

void xgwen_renderer::LoadText( Gwen::Font* pFont, const Gwen::UnicodeString& text )
{
	if(m_font)
		m_font->async_load_glyphs(text.c_str(),text.length());
}

void xgwen_renderer::adjust_pen_pos(Gwen::Point &pos, const Gwen::UnicodeString& text)
{
	float scale = Scale();
	texture_glyph_t *glyph;
	wchar_t ch;
	pos.x += 1;
	pos.y += int(m_font->ascender()*scale+0.5f);
	if(!text.length())
		return;
	ch = text[0];
	glyph = m_font->get_glyph(ch);
	if(glyph && glyph->offset_x<0)
		pos.x -= int(glyph->offset_x-0.5f);
}

void xgwen_renderer::RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text )
{
	xfont *font = (xfont*)m_font;
	assert(font->is_complete());
	font->upload_texture();

	float scale = Scale(), advance=0;
	texture_glyph_t *glyph;
	wchar_t ch=256, prev_ch;
	Gwen::Texture *texture=0;
	unsigned int texture_id;
	glyph_bitmap_t *bm;
	adjust_pen_pos(pos,text);
	for(size_t i=0, cnt=text.length(); i<cnt; ++i)
	{
		prev_ch = ch;
		ch = text[i];
		if(ch==L'\t') 
			continue;
		glyph = font->get_glyph(ch);
		if(!glyph)
			continue;
		bm = glyph->bitmap[GLYPH_NORMAL];
		if(bm)
			texture_id = ((texture_atlas_t*)bm->atlas_id)->id;
		else
			texture_id = 0;
		if(!texture_id) 
		{
			pos.x += int(advance+0.5f);
			advance = 0;
			DrawFilledRect(Gwen::Rect(pos.x,pos.y-int(font->ascender()*scale+0.5f),m_unknown_size.x,m_unknown_size.y));
			pos.x += m_unknown_advance;
			continue;
		}
		if(!texture || *(GLuint*)texture->data != texture_id)
		{
			texture = (Gwen::Texture*)m_fontTextures.get(texture_id);
			if(!texture)
			{
				texture = new Gwen::Texture();
				texture_atlas_t *atlas = (texture_atlas_t*)bm->atlas_id;
				texture->data = new GLuint;
				*(GLuint*)texture->data = atlas->id;
				texture->width = atlas->width;
				texture->height = atlas->height;
				texture->name = Gwen::UnicodeString(L"FontTexture");
				m_fontTextures.add((xdict::key_t)atlas->id,(xdict::value_t)texture);
			}
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,texture_id);
		}
		if(ch<256 && prev_ch<256) 
			advance += font->kerning(prev_ch,ch) * scale;
		pos.x += int(advance+0.5f);
		advance = glyph->advance_x*scale;
		DrawTexturedRect(texture,Gwen::Rect( pos.x+int(bm->offset_x*scale+0.5f), pos.y-int(bm->offset_y*scale+0.5f),\
			int(bm->width*scale+0.5f), int(bm->height*scale+0.5f) ), bm->s0, bm->t0, bm->s1, bm->t1);
	}
}

Gwen::Point xgwen_renderer::MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text )
{
	xfont *font = (xfont*)m_font;
	assert(font->is_complete());

	float scale = Scale();
	wchar_t ch=256, prev_ch;
	texture_glyph_t *glyph;
	Gwen::Point pos(0,0);
	adjust_pen_pos(pos,text);
	for(size_t i=0, cnt=text.length(); i<cnt; ++i) {
		prev_ch = ch;
		ch = text[i];
		if(ch==L'\t')
			continue;
		glyph = font->get_glyph(ch);
		if(!glyph) {
			const wchar_t *s = text.c_str();
			font->async_load_glyphs(s+i,cnt-i);
			glyph = font->get_glyph(ch);
			if(!glyph) {
				pos.x += m_unknown_advance;
				continue;
			}
		}
		if(ch<256 && prev_ch<256) 
			pos.x += int((font->kerning(prev_ch,ch) + glyph->advance_x) * scale + 0.5f);
		else 
			pos.x += int(glyph->advance_x * scale + 0.5f);
	}
	pos.y = int(font->font_height()*scale+0.5f);
	return pos;
}


} // namespace wyc

