#include "fscorepch.h"
#include "wyc/world/com_material.h"

namespace wyc
{

REG_RTTI(xcom_material,xcomponent)

xcom_material::xcom_material()
{
	m_shaderID = 0;
}

void xcom_material::on_destroy()
{
}

bool xcom_material::apply(xrenderer *rd)
{
	const xshader *shader=rd->get_shader();
	if(shader->shader_id()!=m_shaderID) {
		if(!rd->use_shader(m_shaderID))
			return false;
	}
	return true;
}

//-------------------------------------------------------
// diffuse material
//-------------------------------------------------------

REG_RTTI(xmtl_diffuse,xcom_material)


}; // namespace wyc

