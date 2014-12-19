#ifndef __HEADER_WYC_XLAYER_RENDER
#define __HEADER_WYC_XLAYER_RENDER

#include "xobject2d_render.h"

namespace wyc
{

class xlayer_render : public xobject2d_render
{
	USE_EVENT_MAP;
public:
	xlayer_render();
	virtual void on_create(xrenderer *pRenderer, xobjevent *pev);
	virtual void on_destroy();
	virtual void draw(xrenderer *pRenderer);
	virtual void draw_picker(xrenderer *pRenderer);
};

//---------------------------------------------------------------------

class xlayer_pass : public xrenderobj
{
	USE_EVENT_MAP;
protected:
	GLuint m_shader2D;
	xmat4f_t m_mvp;
	xrenderlist m_renderList;
	unsigned m_vieww, m_viewh;
public:
	xlayer_pass();
	virtual void on_create(xrenderer *pRenderer, xobjevent *pev);
	virtual void on_destroy();
	virtual void draw(xrenderer *pRenderer);
	void append(xlayer_render *layer) {
		m_renderList.append(layer);
	}
	void remove(xlayer_render *layer) {
		m_renderList.remove(layer);
	}
	inline size_t size() const {
		return m_renderList.size();
	}
	void ev_mvp_matrix(xobjevent *pev);
};

//---------------------------------------------------------------------

class xlayer_pickpass : public xlayer_pass
{
	USE_EVENT_MAP;
	struct draw_picker {
		inline void operator() (xrenderer *pRenderer, xrenderobj *pobj) {
			((xlayer_render*)pobj)->draw_picker(pRenderer);
		}
	};
	uint32_t *m_pickbuff;
	GLuint m_pickmaps[2];
	unsigned m_gpu2vram;
public:
	xlayer_pickpass();
	virtual void on_create(xrenderer *pRenderer, xobjevent *pev);
	virtual void draw(xrenderer *pRenderer);
private:
	void update_pick_map();
};

}; // namespace wyc

#endif // __HEADER_WYC_XLAYER_RENDER


