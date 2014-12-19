#ifndef __HEADER_WYC_CARD_MATERIAL
#define __HEADER_WYC_CARD_MATERIAL

#include "wyc/util/rect.h"
#include "wyc/util/strutil.h"
#include "wyc/util/hash.h"
#include "wyc/obj/object.h"
#include "wyc/math/transform.h"
#include "wyc/render/vertexbatch.h"
#include "wyc/render/texture.h"
#include "wyc/render/font.h"
#include "wyc/render/text_renderer.h"

#include "card_base.h"

namespace wyc
{

class xcard_renderer : public xobject
{
public:
	xcard_renderer();
	virtual void on_destroy();
	bool load_card_layout(const char *cfg);
	void flush_atlas(xrenderer *rc, bool force=false);
	void draw(xrenderer *rc, const xmat4f_t *camera_transform);
	void add_card(xcard *card);
	void del_card(xcard *card);
	bool is_complete() const;
//-----------------------------------------------------------------------
	void debug_atlas(unsigned screen_w, unsigned screen_h);
	void debug_draw_atlas();
	void debug_next_atlas();
	void debug_add_card();
	void debug_del_card();
	void debug_flush(xrenderer *rc);
private:
	USE_RTTI;
	struct layout_t 
	{
		unsigned width;
		unsigned height;
		float scale;
		xpointer<xfont> def_font;
		xpointer<xtexture> back_face;
		xpointer<xtexture> front_face;
		xrectf_t avatar;
		xrectf_t name;
		xrectf_t cost;
		xrectf_t type;
		xrectf_t rarity;
		xrectf_t info;
		xrectf_t hp;
		xrectf_t strength;
	};
	layout_t m_card_layout;
	xdict m_cards;
	enum LAYOUT_COMPLETION 
	{
		CARD_FONT_DEFAULT = 1,
		CARD_BACKFACE_TEXTURE = 2,
		CARD_FRONTFACE_TEXTURE = 4,
		CARD_MESH_DATA = 8,
		LAYOUT_FBO = 0x20,
		ALL_COMPLETE = CARD_FONT_DEFAULT + CARD_BACKFACE_TEXTURE + CARD_FRONTFACE_TEXTURE + CARD_MESH_DATA + LAYOUT_FBO,
	};
	unsigned m_completion;
	// card mesh data
	GLuint m_vbo_card, m_ibo_card, m_vbo_card_texcoord;
	unsigned m_texbuff_cap;
	unsigned m_texbuff_size;
	size_t m_backface_texcoord;
	// atlas management
	struct atlas_t;
	struct slot_t : public xcard_render_context
	{
		size_t offset;
		unsigned x, y;
		GLfloat u0, v0, u1, v1;
		atlas_t *parent;
		xpointer<xtexture> avatar;
		unsigned avatar_id;
		xpointer<xcard> card;
		virtual void redraw(int flag);
	};
	struct atlas_t
	{
		atlas_t *next;
		xcard_renderer *parent;
		GLuint texture_id;
		unsigned width, height;
		std::vector<slot_t> slots;
		unsigned ref_count;
		unsigned mod_count;
	};
	atlas_t *m_atlas;
	std::vector<slot_t*> m_slot_cache;
	// layout renderer data
	GLuint m_fbo_layout;
	GLuint m_vbo_layout;
	size_t m_vbo_layout_size;
	bool m_flush_atlas;
	// debug draw
	struct {
		GLuint atlas_mesh;
		GLuint atlas_texture;
		xrectf_t atlas_size;
		std::vector<void*> dummy_slots;
	} m_debug_info;
//-----------------------------------------------------------------------
	bool _init_mesh(float w, float h);
	bool _init_atlas();
	atlas_t* _new_atlas();
	void _clear_atlas();
	slot_t* _new_slot();
	void _del_slot(slot_t *s);
	size_t _append_texcoord(GLfloat *coords, size_t size, size_t offset=-1);
	void _on_backface_changed();
	void _resize_layout_buffer(slot_t *slot, size_t &text_size);
	void _set_avatar_vertex(void *buff, slot_t *slot);
	void _render_card_text(xtext_renderer *tr, xcard *card);
	void _set_atlas_dirty();
	void _prepare_card_data(slot_t *slot);
//-----------------------------------------------------------------------
	static void _on_load_layout (xcard_renderer *rc, xresbase *res);
};

inline bool xcard_renderer::is_complete() const 
{
	return ALL_COMPLETE==(m_completion & ALL_COMPLETE);
}

inline void xcard_renderer::_set_atlas_dirty() 
{
	m_flush_atlas =true;
}

}; // namespace wyc

#endif // __HEADER_WYC_CARD_MATERIAL

