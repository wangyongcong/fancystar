#ifndef __HEADER_DEMO_SCENE
#define __HEADER_DEMO_SCENE

#include "wyc/obj/object.h"
#include "wyc/render/renderer.h"
#include "wyc/render/vertexbatch.h"

#include "quad_tree.h"
#include "entity.h"

namespace demo
{

class xscene : public wyc::xobject
{
	USE_RTTI;
	wyc::xquad_tree m_map;
	unsigned m_lod_view;
	unsigned m_pid;
	std::vector<xentity*> m_objects;
	bool m_show_tree, m_show_parent_filled;
public:
	xscene();
	virtual void on_destroy();
	void create(float scn_w, float scn_h);
	void update(float frame_time);
	void draw(wyc::xrenderer *rd);
	void lod_increase() {
		if(m_lod_view+1<m_map_draw.size())
			m_lod_view+=1;
	}
	void lod_decrease() {
		if(m_lod_view>0)
			m_lod_view-=1;
	}
	unsigned width() const {
		return m_map.width();
	}
	unsigned height() const {
		return m_map.height();
	}
	inline unsigned get_next_pid() {
		m_pid += 1;
		return m_pid;
	}
	void add_object (xentity *obj);
	void del_object (xentity *obj);
	xentity* pick (float x, float y, unsigned filter);
	void static_test (xentity *en, unsigned filter, std::vector<xentity*> &list);
	xentity* sweep_test(xentity *en, unsigned filter, const wyc::xvec2f_t &end, float &t, wyc::xvec2f_t &normal);
	bool switch_tree_show() {
		m_show_tree=!m_show_tree;
		return m_show_tree;
	}
	bool switch_filled_show() {
		m_show_parent_filled=!m_show_parent_filled;
		return m_show_parent_filled;
	}
	bool load_map (const char *filename, float scale=5);
	bool save_map (const char *filename, float scale=5);
private:
	GLuint m_map_vao, m_map_vbo;
	std::vector<std::pair<unsigned,unsigned> > m_map_draw;
	bool _create_scene_mesh();
	void _draw_filled_grids();
};

} // namespace demo

#endif // __HEADER_DEMO_SCENE