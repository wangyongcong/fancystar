#ifndef __HEADER_WYC_COM_LAYER
#define __HEADER_WYC_COM_LAYER

#include "wyc/world/com_renderer.h"
#include "wyc/gui/gui_layout.h"

namespace wyc
{

class xcom_layout : public xcom_renderer
{
	USE_RTTI;
	xpointer<xlayout> m_layout;
public:
	xcom_layout() {
		m_layout=wycnew xlayout;
	}
	virtual void on_destroy() {
		m_layout=0;
	}
	virtual void update(double, double) {
		m_layout->update();
	}
	virtual void render(xrenderer *rd) {
		m_layout->render(rd);
	}
	xguiobj* new_element(const char *type, const char *name, float x, float y, float z=0) {
		m_layout->new_element(type,name,x,y,z);
	}
	void del_element(xguiobj *gui_obj) {
		m_layout->del_element(gui_obj);
	}
	xguiobj* get_element(const char *name) {
		m_layout->get_element(name);
	}

};

} // namespace wyc

#endif // __HEADER_WYC_COM_LAYER
