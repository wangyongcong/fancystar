#ifndef __HEADER_WYC_PLAYER
#define __HEADER_WYC_PLAYER

#include <cassert>
#include "wyc/game/glb_game.h"
#include "wyc/render/vertexbatch.h"
#include "obj_base.h"
#include "scope.h"

using namespace wyc;

class xplayer : public xbase_object
{
	USE_RTTI;
	xpointer<xvertex_batch> m_mesh;
	unsigned m_enter;
public:

	xplayer ()
	{
		xressvr *svr=get_resource_server();
		std::string file = "mesh/sphere.json";
		get_resource_path(file);
		m_mesh = (xvertex_batch*)svr->request(xvertex_batch::get_class()->id,file.c_str());
		assert(m_mesh);
		m_transform.scale(0.1f);
		m_transform.set_forward(xvec3f_t(wyc::random(),0,wyc::random()),xvec3f_t(0,1,0));
		m_color.set(0,1,1);
		m_enter=0;
	}

	virtual void on_destroy()
	{
		m_mesh=0;
		xbase_object::on_destroy();
	}

	virtual void draw( const xshader *sh )
	{
		if(!m_mesh->is_complete())
			return;
		GLint uf;
		uf=sh->get_uniform("mat_model");
		if(-1==uf)
			return;
		glUniformMatrix4fv(uf,1,GL_TRUE,m_transform.local2world().data());
		uf=sh->get_uniform("scale");
		if(-1==uf)
			return;
		const xvec3f_t &s = m_transform.scaling();
		glUniform3f(uf,1.0f/s.x,1.0f/s.y,1.0f/s.z);
		uf=sh->get_uniform("mtl_diffuse");
		if(-1==uf)
			return;
		glUniform3f(uf,m_color.x,m_color.y,m_color.z);
		m_mesh->render();
	}

	virtual void on_join_aoi ()
	{
		m_scope=wyc_safecast(xscope,xobject::create_object("xbox_scope"));
		m_scope->create(m_aoi_entity->get_radius().x);
	}

	virtual void aoi_on_enter(xbase_object *obj)
	{
		m_enter+=1;
		if(m_enter==1)
			m_color.set(1,1,0);
	}

	virtual void aoi_on_leave(xbase_object *obj)
	{
		m_enter-=1;
		if(m_enter==0)
			m_color.set(0,1,1);
	}
};

#endif // __HEADER_WYC_PLAYER

