#ifndef __HEADER_WYC_RENDER_TECH
#define __HEADER_WYC_RENDER_TECH

#include "wyc/obj/object.h"

#include "render_object.h"
#include "simple_camera.h"
#include "simple_light.h"

class xrender_tech : public wyc::xobject
{
	USE_RTTI;
protected:
	wyc::xpointer<wyc::xrenderer> m_renderer;
	wyc::xpointer<xsimple_camera> m_camera;
	std::vector<xsimple_light*> m_lights;
	std::vector<wyc::xvec3f_t> m_light_pos;
public:
	xrender_tech()
	{
	}
	virtual void on_destroy()
	{
		for(size_t i=0, count = m_lights.size(); i<count; ++i)
			m_lights[i]->decref();
		m_lights.clear();
	}
	void begin(wyc::xrenderer *rc, xsimple_camera *camera)
	{
		m_renderer = rc;
		m_camera = camera;
		prev_render();
	}
	void end() 
	{
		post_render();
		m_renderer=0;
		m_camera=0;
	}
	virtual void prev_render() {}
	virtual void draw(xrender_object *mesh) {}
	virtual void post_render() {}
	void add_light(xsimple_light *light)
	{
		if(!light)
			return;
		for(size_t i=0, count=m_lights.size(); i<count; ++i)
		{
			if(m_lights[i]==light)
				return;
		}
		light->incref();
		m_lights.push_back(light);
	}
};

#endif // __HEADER_WYC_RENDER_TECH
