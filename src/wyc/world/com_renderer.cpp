#include "fscorepch.h"
#include "wyc/world/com_renderer.h"
#include "wyc/world/gameobj.h"
#include "wyc/world/com_transform.h"
#include "wyc/world/com_mesh.h"

namespace wyc
{

REG_RTTI(xcom_renderer, xcomponent)

xcom_renderer::xcom_renderer()
{
}

void xcom_renderer::before_render(xrenderer *rd)
{
}

void xcom_renderer::render(xrenderer *rd)
{
	xcom_mesh *mesh=gameobj()->get_component<xcom_mesh>();
	if(!mesh || !mesh->visible()) 
		return;
	xcom_material *mtl=gameobj()->get_component<xcom_material>();
	if(mtl && !mtl->apply(rd))
		return;
	xcom_transform *trans=gameobj()->get_transform();
	const xmat4f_t &world=trans->local2world();
	GLint uf=rd->get_shader()->get_uniform("world_tranform");
	if(uf!=-1)
		glUniformMatrix4fv(uf, 1, GL_FALSE, world.data());
	uf=rd->get_shader()->get_uniform("world_rotation");
	if(uf!=-1) 
	{
		xmat3f_t rot=world;
		const xvec3f_t s=trans->scaling();
		rot.elem[0][0]/=s.x;
		rot.elem[1][1]/=s.y;
		rot.elem[2][2]/=s.z;
		glUniformMatrix3fv(uf, 1, GL_FALSE, rot.data());
	}
	xvertex_batch *batch=mesh->get_mesh();
	batch->render();
}

void xcom_renderer::after_render(xrenderer *rd)
{
}


}; // namespace wyc
