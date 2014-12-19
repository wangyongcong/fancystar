#ifndef __HEADER_WYC_RENDER_OBJECT
#define __HEADER_WYC_RENDER_OBJECT

#include "wyc/math/transform.h"
#include "wyc/render/vertexbatch.h"
#include "wyc/render/texture.h"
#include "wyc/obj/object.h"

class xrender_object : public wyc::xobject
{
	USE_RTTI;
protected:
	wyc::xtransform m_transform;
	wyc::xpointer<wyc::xtexture> m_base_map;
public:
	xrender_object()
	{
	}

	virtual void on_destroy()
	{
		m_base_map=0;
	}

	virtual void draw()
	{
	}

	virtual void update_transform(float)
	{
		m_transform.update();
	}
	wyc::xtransform& get_transform() 
	{
		return m_transform;
	}
	const wyc::xtransform& get_transform() const 
	{
		return m_transform;
	}

	void set_base_map(wyc::xtexture *tex)
	{
		m_base_map=tex;
	}
	wyc::xtexture* get_base_map()
	{
		return m_base_map;
	}
};

#endif // __HEADER_WYC_RENDER_OBJECT
