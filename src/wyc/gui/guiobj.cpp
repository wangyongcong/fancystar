#include "fscorepch.h"
#include "wyc/gui/guiobj.h"

namespace wyc
{

REG_RTTI(xguiobj, xobject);

xguiobj::xguiobj() 
{
	m_layer=0;
	m_mesh=0;
	m_pos.zero();
	m_size.zero();
	m_scale=1.0f;
	m_removed=false;
	m_visible=true;
	m_need_reorder=false;
	m_need_rebuild=false;
	m_color=0xFFFFFFFF;
	m_technique=0;
}

void xguiobj::remove()
{
	m_removed=true;
	m_layer->del_element(this);
}

void xguiobj::show()
{
	assert(m_layer);
	m_visible=true;
	m_layer->redraw(this);
}

void xguiobj::hide()
{
	assert(m_layer);
	m_visible=false;
	m_layer->redraw(this);
}

void xguiobj::set_zorder(float z)
{
	assert(m_layer);
	if(!m_need_reorder) {
		m_need_reorder=true;
		m_layer->reset_zorder(this,m_pos.z);
	}
	m_pos.z=z;
}

void xguiobj::redraw() 
{
	assert(m_layer);
	m_layer->redraw(this);
}

void xguiobj::rebuild()
{
	assert(m_layer);
	m_need_rebuild=true;
	m_layer->rebuild(this);
}

xmesh2d* xguiobj::realloc_mesh(xmesh2d *mesh, unsigned vertex_count)
{
	assert(m_layer);
	xtile_buffer *tiles=m_layer->get_tile_buffer();
	if(mesh) {
		if(mesh->vertex_count()>=vertex_count)
			return mesh;
		tiles->free_mesh(mesh);
		wyc_print("realloc mesh [%d]",vertex_count);
	}
	return tiles->alloc_mesh(vertex_count);
}

} // namespace wyc
