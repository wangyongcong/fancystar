#include "fscorepch.h"
#include "wyc/gui/gui_text.h"
#include "wyc/game/glb_game.h"

namespace wyc
{

REG_RTTI(xgui_text,xguiobj)

void xgui_text::on_font_ok (unsigned reqid, xrefobj *obj, xresbase *res)
{
	xgui_text *ui=(xgui_text*)obj;
	xfont *font=(xfont*)res;
	if(reqid==ui->m_wait_font) {
		ui->m_font=font;
		ui->m_wait_font=0;
		ui->load_all_glyphs();
	}
}

void xgui_text::on_glyph_ok (unsigned reqid, xrefobj *obj, xfont*)
{
	xgui_text *ui=(xgui_text*)obj;
	if(reqid==ui->m_wait_glyph) 
		ui->rebuild_text_mesh();
}

xgui_text::xgui_text ()
{
	m_font=0;
	m_cur_font=0;
	m_wait_font=0;
	m_wait_glyph=0;
	m_wrap=false;
	m_scale=1.0f;
	set_technique(strhash("glyph"));
}

void xgui_text::on_destroy()
{
	m_font=0;
	m_cur_font=0;
	m_wait_font=0;
	m_wait_glyph=0;
	m_sortbuff.clear();
}

void xgui_text::build_mesh()
{
	if(!m_cur_font || m_cur_text.empty())
		return;
	build_text_mesh();
}

void xgui_text::build_text_mesh()
{
	//	each glyph is represented with a quad
	// 
	//	(x0,y0)	0 --- 3	
	//			|     |
	//			1 --- 2	(x1,y1)

	size_t len=m_cur_text.size();
	size_t max_chars=xtile_buffer::max_vertex_count()>>2;
	if(len>max_chars)
		len=max_chars;
	alloc_mesh(len*4);
	assert(m_mesh);
	wyc_print("xgui_text::build_text_mesh: [%d] chars",len);
	float pen_x=m_pos.x, pen_y=m_pos.y+m_cur_font->ascender()*m_scale, kerning;
	float x0, y0, x1, y1;
	xvertex2d *vert=&m_mesh->get_vertex(0);
	xfont::glyph_t *glyph;
	GLuint prev_tex_id=0;
	size_t vert_idx=0, prev_vert=0;
	wchar_t prev_char=0;
	bool is_loading;
	m_sortbuff.clear();
	for(size_t i=0; i<len; ++i) {
		if(10==m_cur_text[i]) {
			pen_y+=m_cur_font->font_height()*m_scale;
			pen_x=m_pos.x;
			prev_char=0;
			continue;
		}
		else if(13==m_cur_text[i]) {
			prev_char=0;
			continue;
		}
		else if(9==m_cur_text[i]) {
			glyph=m_cur_font->get_glyph(32,is_loading);
			if(glyph) 
				pen_x+=glyph->advance_x*4*m_scale;
			prev_char=32;
			continue;
		}
		glyph=m_cur_font->get_glyph(m_cur_text[i],is_loading);
		if(!glyph) {
			wchar_t miss_char=m_cur_text[i];
			wyc_warn("xgui_text: glyph not found [%x]",miss_char);
			glyph=m_cur_font->get_glyph(L'?',is_loading);
			if(!glyph)
				continue;
		}

		if(m_font->kerning_available()) {
			if(glyph->charcode<256)
			{
				if(prev_char) {
					kerning=m_cur_font->kerning(prev_char,glyph->charcode);
					pen_x+=kerning*m_scale;
				}
				prev_char=glyph->charcode;
			}
			else prev_char=0;
		}

		// 使用浮点坐标会导致模糊,所以这里要取整
		x0=floor(pen_x+glyph->offset_x*m_scale+0.5f);
		x1=floor(x0+glyph->width*m_scale+0.5f);
		if(m_wrap && x1>=m_pos.x+m_size.x) {
			pen_y+=m_cur_font->font_height()*m_scale;
			if(pen_y>=m_pos.y+m_size.y)
				break;
			pen_x=m_pos.x;
			x0=floor(pen_x+glyph->offset_x*m_scale+0.5f);
			x1=floor(x0+glyph->width*m_scale+0.5f);
		}		
		y0=floor(pen_y-glyph->offset_y*m_scale+0.5f);
		y1=floor(y0+glyph->height*m_scale+0.5f);

		vert->m_pos.set(x0, y0);
		vert->m_texcoord.set(glyph->s0, glyph->t0);
		vert->m_color=m_color;
		++vert;

		vert->m_pos.set(x0,y1);
		vert->m_texcoord.set(glyph->s0, glyph->t1);
		vert->m_color=m_color;
		++vert;

		vert->m_pos.set(x1,y1);
		vert->m_texcoord.set(glyph->s1, glyph->t1);
		vert->m_color=m_color;
		++vert;

		vert->m_pos.set(x1,y0);
		vert->m_texcoord.set(glyph->s1, glyph->t0);
		vert->m_color=m_color;
		++vert;

		pen_x+=glyph->advance_x*m_scale;
		if (prev_tex_id!=((texture_atlas_t*)glyph->id)->id) {
			if(prev_tex_id) 
				m_sortbuff.push_back(xvec3i_t(prev_tex_id,prev_vert,vert_idx));
			prev_tex_id=((texture_atlas_t*)glyph->id)->id;
			prev_vert=vert_idx;
		}
		vert_idx+=4;
	}
	assert(vert<=&m_mesh->get_vertex(0)+m_mesh->vertex_count());
	assert(vert_idx<=m_mesh->vertex_count());
	if(prev_tex_id) 
		m_sortbuff.push_back(xvec3i_t(prev_tex_id,prev_vert,vert_idx));
	for(size_t i=0; i<m_sortbuff.size(); ++i) {
		prev_tex_id=m_sortbuff[i].x;
		for(size_t j=i+1; j<m_sortbuff.size(); ++j) {
			if(prev_tex_id==unsigned(m_sortbuff[j].x)) {
				++i;
				if(i!=j) 
					std::swap(m_sortbuff[j],m_sortbuff[i]);
			}
		}
	}
	m_mesh->commit_vertex();
	redraw();
}

void xgui_text::draw()
{
	if(!m_mesh) 
		return;
	GLuint tex_id;
	size_t beg, end, len;
	std::vector<unsigned> ibuff;
	for(size_t i=0; i<m_sortbuff.size(); ++i) {
		tex_id=m_sortbuff[i].x;
		beg=m_sortbuff[i].y;
		end=m_sortbuff[i].z;
		len=end-beg;
		assert(0==(len&3));
		ibuff.reserve(len*6);
		for(; beg!=end; beg+=4) {
			ibuff.push_back(beg);
			ibuff.push_back(beg+1);
			ibuff.push_back(beg+2);
			ibuff.push_back(beg);
			ibuff.push_back(beg+2);
			ibuff.push_back(beg+3);
		}
		m_mesh->draw(&ibuff[0],ibuff.size(),tex_id,get_technique());
		ibuff.clear();
	}
}

void xgui_text::set_font (const char *font_name)
{
	assert(font_name);
	m_font_name=font_name;
	m_wait_glyph=0;
	xressvr *svr=get_resource_server();
	xresbase *res=svr->request(xfont::get_class()->id,font_name,m_wait_font,this,xgui_text::on_font_ok);
	if(res) xgui_text::on_font_ok(m_wait_font,this,res);
}

void xgui_text::set_text (const wchar_t *text)
{
	m_text=text;
	if(!m_wait_font && m_font) 
		load_all_glyphs();
}

void xgui_text::set_text (const wchar_t *text, size_t pos, size_t count)
{
	size_t len=wcslen(text);
	if(pos<len)
		m_text.assign(text+pos,count);
	else
		m_text.clear();
	if(!m_wait_font && m_font) 
		load_all_glyphs();
}

void xgui_text::set_text (const std::wstring &text)
{
	m_text=text;
	if(!m_wait_font && m_font) 
		load_all_glyphs();
}

void xgui_text::set_text (const std::wstring &text, size_t pos, size_t count)
{
	m_text.assign(text,pos,count);
	if(!m_wait_font && m_font) 
		load_all_glyphs();
}

void xgui_text::load_all_glyphs()
{
	if(m_wait_font || !m_font)
		return;
	m_font->prepare_glyph(m_text.c_str());
	m_wait_glyph=m_font->flush(this,xgui_text::on_glyph_ok);
	if(!m_wait_glyph)
	{
		rebuild_text_mesh();
	}
}

void xgui_text::rebuild_text_mesh()
{
	m_font->upload_atlas();
	m_cur_font=m_font;
	size_t max_chars=xtile_buffer::max_vertex_count()>>2;
	if(m_text.size()>max_chars) {
		m_cur_text.reserve(max_chars);
		m_cur_text.assign(m_text,0,max_chars-3);
		m_cur_text+=L"...";
	}
	else 
		m_cur_text=m_text;
	rebuild();
}

} // namespace wyc

