#include "fscorepch.h"
#include "wyc/world/com_mesh.h"
#include "wyc/world/glb_world.h"

namespace wyc
{

REG_RTTI(xcom_mesh,xcomponent)

void xcom_mesh::on_load_mesh_ok(xrefobj *notifier, xresbase *res)
{
	assert(notifier);
	assert(res);
	xcom_mesh *com=(xcom_mesh*)notifier;
	if(com->m_batch!=res) 
		return;
	if(!res->is_complete()) 
	{
		com->m_batch=0;
		wyc_print("xcom_mesh::on_load_mesh_ok: fail!");
	}
}

xcom_mesh::xcom_mesh()
{
}

void xcom_mesh::on_destroy()
{
	m_batch=0;
}

void xcom_mesh::load_mesh(const char *name)
{
	xressvr *svr=get_resource_server();
	m_batch=(xvertex_batch*)svr->async_request(xvertex_batch::get_class()->id,name,this,xcom_mesh::on_load_mesh_ok);
}

}; // namespace wyc
