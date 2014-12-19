#include "fscorepch.h"

#include "tech_lighting_base.h"

namespace wyc
{

class xtech_parallax : public xtech_lighting_base
{
	USE_RTTI;
public:
	xtech_parallax();
	virtual void before_draw(const xshader *shader, xsimple_mesh *mesh);
	virtual void post_render();
};

} // namespace wyc
