#include "fscorepch.h"
#include "wyc/gui/gui_rtinfo.h"
#include "wyc/game/glb_game.h"
#include "wyc/util/strutil.h"

namespace wyc
{

REG_RTTI(xgui_rtinfo,xguiobj)

xgui_rtinfo::xgui_rtinfo()
{
	m_mesh_info=0;
	m_size_tag=64;
	m_size_info=96;
	m_count_per_row=1;
	m_chars_tag=0;
	m_chars_info=0;
	m_rc_tag.set_custom_renderer(&_draw_text_mesh);
	m_rc_info.set_custom_renderer(&_draw_text_mesh);
	memset(&m_debug_stat,0,sizeof(m_debug_stat));
}

void xgui_rtinfo::on_destroy()
{
	m_font=0;
	m_entries.clear();
}

void xgui_rtinfo::build_mesh()
{
	if(!m_font) {
		// render something when font is not ready
		return;
	}
	if(!m_chars_tag) {
		// nothing to display
		return;
	}
	m_font->upload_texture();
	if(m_build_tag) 
		build_tag_mesh();
	build_info_mesh();
	redraw();
	// update debug statistics
	m_debug_stat.m_draw+=1;
	m_debug_stat.m_vert_cost=0;
	if(m_mesh)
		m_debug_stat.m_vert_cost+=m_mesh->vertex_count();
	if(m_mesh_info)
		m_debug_stat.m_vert_cost+=m_mesh_info->vertex_count();
	m_debug_stat.m_vert_used=4*(m_chars_tag+m_chars_info);
}

void xgui_rtinfo::draw()
{
	if(m_font) 
	{
		struct {
			xmesh2d *mesh;
			unsigned technique;
		} context;
		context.technique = get_technique();
		if(m_mesh) {
			context.mesh = m_mesh;
			m_rc_tag.set_render_context(&context);
			m_rc_tag.render();
		}
		if(m_mesh_info) {
			context.mesh = m_mesh_info;
			m_rc_info.set_render_context(&context);
			m_rc_info.render();
		}

	}
}

void xgui_rtinfo::set_font(xfont *font)
{
	if(!font->is_complete())
		return;
	m_font=font;
	reload_all_glyphs();
}

void xgui_rtinfo::reload_all_glyphs()
{
	assert(m_font);
	unsigned loading=m_font->async_load_glyphs(L" :?");
	for(size_t i=0, cnt=m_entries.size(); i<cnt; ++i) 
	{
		entry_t &en=m_entries[i];
		loading+=m_font->async_load_glyphs(en.m_tag.c_str());
		loading+=m_font->async_load_glyphs(en.m_info.c_str());
	}
	m_debug_stat.m_req_glyph+=loading;
	rebuild();
}

void xgui_rtinfo::set_entry(const wchar_t *tag, const wchar_t *info, uint32_t tag_color, uint32_t info_color)
{
	bool new_tag=true;
	for(entry_list_t::iterator iter=m_entries.begin(), end=m_entries.end(); iter!=end; ++iter) 
	{
		if(iter->m_tag==tag) 
		{
			m_chars_info-=iter->m_info.size();
			iter->m_info=info;
			m_chars_info+=iter->m_info.size();
			new_tag=false;
			break;
		}
	}
	if(new_tag)
	{
		m_entries.resize(m_entries.size()+1);
		entry_t &en=m_entries.back();
		en.m_tag=tag;
		en.m_info=info;
		en.m_color_tag=tag_color;
		en.m_color_info=info_color;
		m_build_tag=true;
		m_chars_tag+=en.m_tag.size()+1;
		m_chars_info+=en.m_info.size();
	}
	if(m_font) {
		unsigned loading=0;
		if(new_tag) 
			loading+=m_font->async_load_glyphs(tag);
		loading+=m_font->async_load_glyphs(info);
		m_debug_stat.m_req_glyph+=loading;
		rebuild();
	}
}

void xgui_rtinfo::del_entry(const wchar_t *tag)
{
	entry_list_t::iterator iter=m_entries.begin(), end=m_entries.end();
	for(; iter!=end; ++iter) 
	{
		if(iter->m_tag==tag) 
		{
			m_chars_tag-=iter->m_tag.size()+1;
			m_chars_info-=iter->m_info.size();
			m_entries.erase(iter);
			rebuild();
			return;
		}
	}
}

void xgui_rtinfo::set_entry_color(const wchar_t *tag, uint32_t tag_color, uint32_t info_color)
{
	entry_list_t::iterator iter=m_entries.begin(), end=m_entries.end();
	for(; iter!=end; ++iter) 
	{
		if(iter->m_tag==tag) 
		{
			if(iter->m_color_tag!=tag_color) {
				iter->m_color_tag=tag_color;
				m_build_tag=true;
			}
			iter->m_color_info=info_color;
			rebuild();
			return;
		}
	}
}

void xgui_rtinfo::build_tag_mesh()
{
	m_build_tag=false;
	size_t vert_count=m_chars_tag*4, col=0;
	alloc_mesh(vert_count);
	m_rc_tag.attach_buffer(&m_mesh->get_vertex(0),sizeof(xvertex2d)*vert_count);

	float pen_x, pen_y, x, x_advance=m_size_tag+m_size_info;
	pen_y=m_pos.y+m_font->ascender()*m_scale, x=m_pos.x;
	entry_list_t::iterator iter, end=m_entries.end();
	for(iter=m_entries.begin(); iter!=end; ++iter)
	{
		pen_x=x;
		m_rc_tag.draw_text(pen_x,pen_y,iter->m_tag.c_str(),m_font,iter->m_color_tag,m_scale,0);
		m_rc_tag.draw_text(pen_x,pen_y,L":",m_font,iter->m_color_tag,m_scale,0);
		col+=1;
		if(col>=m_count_per_row) {
			x=m_pos.x;
			pen_y+=m_font->font_height()*m_scale;
			col=0;
		}
		else x+=x_advance;
	}
	m_mesh->commit_vertex();
	m_debug_stat.m_build_tag+=1;
}

void xgui_rtinfo::build_info_mesh()
{
	size_t vert_count=m_chars_info*4, col=0;
	m_mesh_info=realloc_mesh(m_mesh_info,vert_count);
	m_rc_info.attach_buffer(&m_mesh_info->get_vertex(0),sizeof(xvertex2d)*vert_count);

	float pen_x, pen_y, x, x_advance=m_size_tag+m_size_info;
	pen_y=m_pos.y+m_font->ascender()*m_scale, x=m_pos.x+m_size_tag+4;
	entry_list_t::iterator iter, end=m_entries.end();
	for(iter=m_entries.begin(); iter!=end; ++iter)
	{
		pen_x=x;
		m_rc_info.draw_text(pen_x,pen_y,iter->m_info.c_str(),m_font,iter->m_color_info,m_scale,0);
		col+=1;
		if(col>=m_count_per_row) {
			x=m_pos.x+m_size_tag+4;
			pen_y+=m_font->font_height()*m_scale;
			col=0;
		}
		else x+=x_advance;
	}
	m_mesh_info->commit_vertex();
	m_debug_stat.m_build_info+=1;
}

void xgui_rtinfo::_draw_text_mesh(void *ctx, GLuint texture_id, GLushort *index_buffer, size_t index_count)
{
	typedef struct {
		xmesh2d *mesh;
		unsigned technique;
	} context_t;
	((context_t*)ctx)->mesh->draw(index_buffer,index_count,texture_id,((context_t*)ctx)->technique);
}

void xgui_rtinfo::debug_update()
{
	std::wstring s;
	wyc::uint2str(s,m_debug_stat.m_build_tag);
	set_entry(L"build tag",s.c_str(),0xFF00AA00,0xFFFF00FF);
	wyc::uint2str(s,m_debug_stat.m_build_info);
	set_entry(L"build info",s.c_str(),0xFF00AA00,0xFFFF00FF);
	wyc::uint2str(s,m_debug_stat.m_req_glyph);
	set_entry(L"req glyph",s.c_str(),0xFF00AA00,0xFFFF00FF);
	wyc::uint2str(s,m_debug_stat.m_draw);
	set_entry(L"redraw",s.c_str(),0xFF00AA00,0xFFFF00FF);
	wyc::format(s,L"%d/%d",m_debug_stat.m_vert_used,m_debug_stat.m_vert_cost);
	set_entry(L"vertices",s.c_str(),0xFF00AA00,0xFFFF00FF);
}

}; // namespace wyc

