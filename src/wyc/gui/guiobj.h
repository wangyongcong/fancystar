#ifndef __HEADER_WYC_GUIOBJ
#define __HEADER_WYC_GUIOBJ

#include "wyc/math/vecmath.h"
#include "wyc/gui/gui_layout.h"

namespace wyc
{

enum BLIT_TYPE 
{
	NO_BLIT=0,
	BLIT_HV,
	BLIT_H3,
	BLIT_V3,
	BLIT_33,
	MAX_BLIT_TYPE,
};

class xguiobj : public xobject
{
	USE_RTTI;
	friend class xlayout;
	xlayout *m_layer;
	std::string m_name;
	unsigned m_technique;
	bool m_removed;
	bool m_visible;
	bool m_need_reorder;
	bool m_need_rebuild;
protected:
	xmesh2d *m_mesh;
	xvec3f_t m_pos;
	xvec2f_t m_size;
	float m_scale;
	uint32_t m_color;
public:
	xguiobj();
	virtual void build_mesh() {}
	virtual void draw() {}
	void show();
	void hide();
	inline bool visible() const
	{
		return m_visible;
	}
	void remove();
	bool is_removed() const 
	{
		return m_removed;
	}
	xlayout* get_layer() 
	{
		return m_layer;
	}
	const std::string& name() const
	{
		return m_name;
	}
	void set_position(float x, float y)
	{
		m_pos.x=x; m_pos.y=y;
		rebuild();
	}
	void get_position(float &x, float &y) const
	{
		x=m_pos.x, y=m_pos.y;
	}
	void set_size(float w, float h)
	{
		m_size.set(w,h);
		rebuild();
	}
	const xvec2f_t& get_size() const
	{
		return m_size;
	}
	void get_size(float &w, float &h) const
	{
		w=m_size.x, h=m_size.y;
	}
	void set_zorder(float z);
	float get_zorder() const 
	{
		return m_pos.z;
	}
	float get_scale() const
	{
		return m_scale;
	}
	void set_scale(float s)
	{
		m_scale=s;
		rebuild();
	}
	void set_color (uint32_t rgba)
	{
		m_color=rgba;
		rebuild();
	}
	void set_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
	{
		set_color((alpha<<24)|(red<<16)|(green<<8)|blue);
	}
	uint32_t get_color() const 
	{
		return m_color;
	}
	unsigned get_technique() const
	{
		return m_technique;
	}
	void set_technique(unsigned tech)
	{
		m_technique=tech;
	}
	void redraw();
	void rebuild();
protected:
	xmesh2d* realloc_mesh(xmesh2d *mesh, unsigned vertex_count);
	inline void alloc_mesh(unsigned vertex_count)
	{
		m_mesh=realloc_mesh(m_mesh,vertex_count);
	}
	bool need_rebuild() const {
		return m_need_rebuild;
	}
	void reset_rebuild_bit() {
		m_need_rebuild=false;
	}
	bool need_reorder() const {
		return m_need_reorder;
	}
	void reset_reorder_bit() {
		m_need_reorder=false;
	}
};

}; // namespace wyc

#endif // __HEADER_WYC_GUIOBJ
