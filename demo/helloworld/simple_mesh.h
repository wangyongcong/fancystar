#ifndef __HEADER_WYC_SIMPLE_MESH
#define __HEADER_WYC_SIMPLE_MESH

#include <string>
#include <cassert>

#include "wyc/render/vertexbatch.h"
#include "wyc/game/glb_game.h"
#include "render_object.h"

class xsimple_mesh : public xrender_object
{
	USE_RTTI;
	std::string m_file;
	wyc::xpointer<wyc::xvertex_batch> m_batch;
	wyc::xpointer<wyc::xtexture> m_normal_map;
	wyc::xpointer<wyc::xtexture> m_height_map;
	wyc::xvec3f_t m_diffuse, m_specular;
	float m_smoothness;
public:
	xsimple_mesh() : xrender_object()
	{
		m_diffuse.set(1,1,1);
		m_specular.set(1,1,1);
		m_smoothness=8;
	}

	virtual void on_destroy()
	{
		m_batch=0;
		m_normal_map=0;
		m_height_map=0;
		xrender_object::on_destroy();
	}

	bool load(const char *file_name)
	{
		wyc::xressvr *svr=wyc::get_resource_server();
		m_file=file_name;
		m_batch=(wyc::xvertex_batch*)svr->async_request(wyc::xvertex_batch::get_class()->id,file_name,this,
			(wyc::xressvr::resource_callback_t)xsimple_mesh::_load_mesh_callback);
		return m_batch!=0;
	}

	void on_load_ok()
	{
	}

	void on_load_fail()
	{
		wyc_error("Fail to load mesh file: %s",m_file.c_str());
		m_batch=0;
		m_file.clear();
	}

	virtual void draw ()
	{
		if(!(m_batch && m_batch->is_complete()))
			return;
		m_batch->render();
		assert(glGetError()==GL_NO_ERROR);
	}

	static void _load_mesh_callback(xsimple_mesh *self, wyc::xvertex_batch *res)
	{
		if(res==self->m_batch) {
			if(res->is_complete())
				self->on_load_ok();
			else
				self->on_load_fail();
		}
	}

	void set_normal_map(wyc::xtexture *tex)
	{
		m_normal_map=tex;
	}
	wyc::xtexture* get_normal_map()
	{
		return m_normal_map;
	}

	void set_height_map(wyc::xtexture *tex)
	{
		m_height_map = tex;
	}
	wyc::xtexture* get_height_map()
	{
		return m_height_map;
	}

	void set_material(const wyc::xvec3f_t &diffuse,  const wyc::xvec3f_t &specular, float smoothness)
	{
		m_diffuse=diffuse;
		m_specular=specular;
		m_smoothness=smoothness;
	}
	const wyc::xvec3f_t& get_diffuse() const {
		return m_diffuse;
	}
	const wyc::xvec3f_t& get_specular() const {
		return m_specular;
	}
	const float get_smoothness() const {
		return m_smoothness;
	}

	wyc::xvertex_batch* get_batch() {
		return m_batch;
	}
};

#endif // __HEADER_WYC_SIMPLE_MESH
