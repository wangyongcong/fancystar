#ifndef __HEADER_WYC_XOBJECT2D_RENDER
#define __HEADER_WYC_XOBJECT2D_RENDER

#include "xrenderobj.h"
#include "xvecmath.h"
#include "xtexture.h"
#include "tilebuffer.h"

namespace wyc
{

class xobject2d_render : public xrenderobj
{
	USE_EVENT_MAP;
protected:
	struct zorder_compare {
		bool operator() (const xobject2d_render *l, const xobject2d_render *r) const {
			if(l->m_pos.z!=r->m_pos.z)
				return l->m_pos.z<r->m_pos.z;
			return l<r;
		}
	};
	typedef std::set<xobject2d_render*,zorder_compare> child_list_t;
	xobject2d_render *m_parent;
	child_list_t m_children;
	xvec3f_t m_pos;
	xvec2f_t m_size;
	float m_rotate;
	uint32_t m_color[4];
	xvec2f_t m_kp;
	uint32_t m_pickCode;
	float m_alphaFilter;
	xpointer<xmesh2d> m_spMesh;
	xpointer<xtexture> m_spImage;
	struct xobjframe {
		xpointer<xmesh2d> m_spFrameMesh;
		xpointer<xtexture> m_spFrameImage;
		uint32_t m_frameCode[8];
		float m_frameWidth;
	};
	xobjframe *m_pFrame;
	xtile_buffer *m_pTilebuffer;
	unsigned m_flag;
	enum {
		OBJ2D_BLIT_MASK=0xF,
		OBJ2D_CHANGED_MASK=0xF0,
		OBJ2D_MESH_CHANGED=0x10,
		OBJ2D_IMAGE_CHANGED=0x20,
		OBJ2D_COLOR_CHANGED=0x40,
		OBJ2D_PICK_MASK=0xF00,
		OBJ2D_PICK_SHIFT=8,
		OBJ2D_SHOW_FRAME=0x1000,
		OBJ2D_PICK_FRAME=0x2000,
	};
	struct xrender_context {
		GLuint m_vbo;
		GLuint m_tex;
		GLenum m_wrap_s, m_wrap_t;
		GLint m_uniTrans;
		GLint m_uniPick;
		int m_pickType;
	};
public:
	xobject2d_render();
	virtual void on_destroy();
	const xvec3f_t& get_pos() const;
	const xvec2f_t& get_size() const;
	const xvec2f_t& keypoint() const;
	void get_world_pos(xvec2f_t &pos) const;
	float get_rotate() const;
protected:
	inline bool pickable() const {
		return m_pickCode!=0;
	}
	inline PICK_TYPE pick_type() const {
		return PICK_TYPE((m_flag&OBJ2D_PICK_MASK)>>OBJ2D_PICK_SHIFT);
	}
	xtile_buffer* get_tile_buffer();
	void r_draw2d(xrender_context &rc);
	void r_picker(xrender_context &rc);
	void r_prepare_draw_mesh(xrender_context &rc, xmesh2d *pmesh);
	void create_frame();
	void destroy_frame();
	void update_frame_position();
	void update_frame_texture();
	void update_frame_color(unsigned idx, uint32_t color);
	void update_frame_color(unsigned count, uint32_t *pcolor);
	//------------------------------------------------------
	// events handler
	//------------------------------------------------------
	void ev_set_parent(xobjevent *pev);
	void ev_transform(xobjevent *pev);
	void ev_set_color(xobjevent *pev);
	void ev_set_image(xobjevent *pev);
	void ev_set_keypoint(xobjevent *pev);
	void ev_set_pick(xobjevent *pev);
	void ev_prepare_draw(xobjevent *pev);
	void ev_set_visible(xobjevent *pev);
	void ev_show_frame(xobjevent *pev);
	void ev_hide_frame(xobjevent *pev);
	void ev_enable_frame(xobjevent *pev);
	void ev_disable_frame(xobjevent *pev);
	void ev_set_frame_color(xobjevent *pev);
	//------------------------------------------------------
	enum XOBJ2D_MESH_BUILDFLAG {
		BUILD_ALL=0xF,
		BUILD_POSITION=0x1,
		BUILD_COLOR=0x2,
		BUILD_TEXTURE=0x4,
		BUILD_INDEX=0x8,
	};
	static void mesh_no_blit(xobject2d_render*,int);
	static void mesh_blit_h (xobject2d_render*,int);
	static void mesh_blit_v (xobject2d_render*,int);
	static void mesh_blit_hv(xobject2d_render*,int);
	static void mesh_blit_h3(xobject2d_render*,int);
	static void mesh_blit_v3(xobject2d_render*,int);
	static void mesh_blit_33(xobject2d_render*,int);
	static void build_mesh_quad(xobject2d_render*);
};

inline const xvec3f_t& xobject2d_render::get_pos() const {
	return m_pos;
}

inline const xvec2f_t& xobject2d_render::get_size() const {
	return m_size;
}

inline float xobject2d_render::get_rotate() const {
	return m_rotate;
}

inline const xvec2f_t& xobject2d_render::keypoint() const {
	return m_kp;
}

}; // namespace wyc

#endif // __HEADER_WYC_XOBJECT2D_RENDER


