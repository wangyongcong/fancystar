#include "fscorepch.h"
#include "wyc/render/text_renderer.h"

namespace wyc
{

xtext_renderer::xtext_renderer()
{
	m_vbuffer=0;
	m_vbuffer_size=0;
	m_vert_end=0;
	m_vert_count=0;
	m_custom_renderer=0;
	m_custom_context=0;
	m_need_sort=false;
}

xtext_renderer::~xtext_renderer()
{
}

void xtext_renderer::attach_buffer(void *vert_buff, size_t size)
{
	m_vbuffer=(xvertex2d*)vert_buff;
	m_vbuffer_size=size;
	m_vert_end=m_vbuffer+(size/sizeof(xvertex2d));
	m_vert_count=0;
	m_draw_buffer.clear();
	m_need_sort=false;
}

void xtext_renderer::detach_buffer()
{
	m_vbuffer=0;
	m_vbuffer_size=0;
	m_vert_count=0;
}

void xtext_renderer::clear_buffer()
{
	m_vert_count=0;
	m_draw_buffer.clear();
	m_need_sort=false;
}

void xtext_renderer::draw_text(float &pen_x, float &pen_y, const wchar_t *text, xfont *font, uint32_t color, float scale, const xrectf_t *bounding_box, int style)
{
	//	each glyph is represented with a quad
	// 
	//	(x0,y0)	0 --- 3	
	//			|     |
	//			1 --- 2	(x1,y1)

	if(!m_vbuffer) return;
	xvertex2d *vert=m_vbuffer+m_vert_count;
	texture_glyph_t *glyph;
	GLuint prev_tex;
	xvec3i_t *draw_rec;
	if(m_draw_buffer.empty()) {
		draw_rec=0;
		prev_tex=0;
	}
	else {
		draw_rec=&m_draw_buffer.back();
		prev_tex=draw_rec->x;
	}
	float x0, y0, x1, y1, kerning;
	glyph_bitmap_t *bitmap;
	wchar_t prev_char=0;
	for(const wchar_t *c=text; *c && vert<=m_vert_end; ++c)
	{
		if(10==*c) {
			if(bounding_box) {
				pen_y+=font->font_height()*scale;
				pen_x=bounding_box->left;
				prev_char=0;
			}
			continue;
		}
		else if(13==*c) {
			prev_char=0;
			continue;
		}
		else if(9==*c) {
			glyph=font->get_glyph(32);
			if(glyph) 
				pen_x+=glyph->advance_x*4*scale;
			prev_char=32;
			continue;
		}

		glyph=font->get_glyph(*c);
		if(!glyph) {
			glyph=font->get_glyph(L'?');
			if(!glyph)
				continue;
		}

		if(glyph->charcode-L'a'<26 || glyph->charcode-L'A'<26)
		{
			if(prev_char) {
				kerning=font->kerning(prev_char,glyph->charcode);
				pen_x+=kerning*scale;
			}
			prev_char=glyph->charcode;
		}
		else prev_char=0;

		assert(style<GLYPH_STYLE_COUNT);
		bitmap=glyph->bitmap[style];
		if(!bitmap || !bitmap->atlas_id)
			continue;

		// 使用浮点坐标会导致模糊,所以这里要取整
		x0=floor(pen_x+bitmap->offset_x*scale+0.5f);
		x1=floor(x0+bitmap->width*scale+0.5f);
		if(bounding_box && x1>=bounding_box->right) {
			pen_y+=font->font_height()*scale;
			if(pen_y>=bounding_box->bottom)
				break;
			pen_x=bounding_box->left;
			x0=floor(pen_x+bitmap->offset_x*scale+0.5f);
			x1=floor(x0+bitmap->width*scale+0.5f);
		}		
		y0=floor(pen_y-bitmap->offset_y*scale+0.5f);
		y1=floor(y0+bitmap->height*scale+0.5f);

		vert->m_pos.set(x0, y0);
		vert->m_texcoord.set(bitmap->s0, bitmap->t0);
		vert->m_color=color;
		++vert;

		vert->m_pos.set(x0,y1);
		vert->m_texcoord.set(bitmap->s0, bitmap->t1);
		vert->m_color=color;
		++vert;

		vert->m_pos.set(x1,y1);
		vert->m_texcoord.set(bitmap->s1, bitmap->t1);
		vert->m_color=color;
		++vert;

		vert->m_pos.set(x1,y0);
		vert->m_texcoord.set(bitmap->s1, bitmap->t0);
		vert->m_color=color;
		++vert;

		pen_x+=glyph->advance_x*scale;

		if (prev_tex!=((texture_atlas_t*)bitmap->atlas_id)->id) {
			prev_tex=((texture_atlas_t*)bitmap->atlas_id)->id;
			m_draw_buffer.push_back(xvec3i_t(prev_tex,m_vert_count,4));
			draw_rec=&m_draw_buffer.back();
		}
		else draw_rec->z+=4;

		m_vert_count+=4;
	}
	m_need_sort=true;
}

void xtext_renderer::sort_mesh()
{
	GLuint prev_tex;
	for(size_t i=0; i<m_draw_buffer.size(); ++i) {
		prev_tex=m_draw_buffer[i].x;
		for(size_t j=i+1; j<m_draw_buffer.size(); ++j) {
			if(prev_tex==unsigned(m_draw_buffer[j].x)) {
				++i;
				if(i!=j) 
					std::swap(m_draw_buffer[j],m_draw_buffer[i]);
			}
		}
	}
}

void xtext_renderer::render()
{
	if(m_need_sort) 
	{
		m_need_sort=false;
		sort_mesh();
	}
	GLuint tex_id;
	size_t beg, len;
	std::vector<GLushort> ibuff;
	for(size_t i=0; i<m_draw_buffer.size(); ++i) {
		tex_id=m_draw_buffer[i].x;
		beg=m_draw_buffer[i].y;
		len=m_draw_buffer[i].z;
		assert(0==(len&3));
		ibuff.reserve((len>>2)*6);
		for(size_t j=4; j<=len; j+=4, beg+=4) {
			ibuff.push_back(beg);
			ibuff.push_back(beg+1);
			ibuff.push_back(beg+2);
			ibuff.push_back(beg);
			ibuff.push_back(beg+2);
			ibuff.push_back(beg+3);
		}
		if(m_custom_renderer) 
			m_custom_renderer(m_custom_context, tex_id, &ibuff[0], ibuff.size());
		else {
			glBindTexture(GL_TEXTURE_2D,tex_id);
			glDrawElements(GL_TRIANGLES,ibuff.size(),GL_UNSIGNED_SHORT,(void*)(&ibuff[0]));
		}
		ibuff.clear();
	}
}

}; // namespace wyc
