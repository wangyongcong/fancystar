#include "fscorepch.h"
#include "wyc/world/com_light.h"
#include "wyc/world/gameobj.h"

namespace wyc
{

REG_RTTI(xcom_light,xcomponent)

xcom_light::xcom_light()
{
	m_intensity=1;
	m_color.set(1,1,1);
}

void xcom_light::apply_lighting(int idx, xvec3f_t *pos, xvec3f_t *intensity) 
{
	xcom_transform *trans = gameobj()->get_transform();
	pos[idx]=trans->position()*trans->local2world();
	intensity[idx]=m_color*m_intensity;
}


}; // namespace wyc
