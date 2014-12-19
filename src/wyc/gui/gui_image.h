#ifndef __HEADER_WYC_GUI_IMAGE
#define __HEADER_WYC_GUI_IMAGE

#include "wyc/render/texture.h"
#include "wyc/gui/guiobj.h"

namespace wyc
{

class xgui_image : public xguiobj
{
	USE_RTTI;
	xpointer<xtexture> m_tex;
	std::string m_tex_name;
	std::string m_img_name;
	BLIT_TYPE m_blit;
	typedef void (xgui_image::*mesh_builder_t)(void);
	static mesh_builder_t ms_mesh_builder[MAX_BLIT_TYPE];
public:
	xgui_image();
	virtual void on_destroy();
	virtual void build_mesh();
	virtual void draw();
	void set_image (const char *image_name, BLIT_TYPE blit=NO_BLIT);
	void set_image (const char *imageset, const char *image_name, BLIT_TYPE blit=NO_BLIT);
	void image_size(unsigned &w, unsigned &h) const;
protected:
	static void on_image_ok(xrefobj *obj, xresbase *res);
	void _mesh_no_blit();
	void _mesh_blit_hv();
	void _mesh_blit_h3();
	void _mesh_blit_v3();
	void _mesh_blit_33();
};

inline void xgui_image::image_size(unsigned &w, unsigned &h) const
{
	if(m_tex) {
		w=m_tex->image_width();
		h=m_tex->image_height();
	}
	else w=h=0;
}

}; // namespace wyc

#endif // __HEADER_WYC_GUI_IMAGE
