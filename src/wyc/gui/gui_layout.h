#ifndef __HEADER_WYC_GUI_LAYER
#define __HEADER_WYC_GUI_LAYER

#include "wyc/util/hash.h"
#include "wyc/render/tilebuffer.h"

namespace wyc
{

class xguiobj;

class xlayout : public xobject
{
	USE_RTTI;
	xpointer<xtile_buffer> m_tiles;
	std::vector<xguiobj*> m_elements;
	std::vector<std::pair<xguiobj*,float> > m_elem_reorder;
	xdict m_name2obj;
	unsigned m_redraw_count;
	bool m_redraw;
	bool m_remove_elem;
	bool m_rebuild;
public:
	xlayout();
	virtual void on_destroy();
	void update();
	void render(xrenderer *rd);
	xguiobj* new_element(const char *type, const char *name, float x, float y, float z=0);
	void del_element(xguiobj *gui_obj);
	xguiobj* get_element(const char *name);
	void redraw(xguiobj *gui_obj);
	void rebuild(xguiobj *gui_obj);
	void reset_zorder(xguiobj *gui_obj, float z);
	// inner used only
	xtile_buffer* get_tile_buffer();
protected:
	void update_elements();
	void remove_elements();
	void reorder_elements();
	void _del_element_impl(xguiobj *gui_obj);
};

inline void xlayout::redraw(xguiobj*)
{
	m_redraw=true;
}

inline void xlayout::rebuild(xguiobj*) 
{
	m_rebuild=true;
}

inline xtile_buffer* xlayout::get_tile_buffer()
{
	return m_tiles;
}

}; // namespace wyc

#endif // __HEADER_WYC_GUI_LAYER
