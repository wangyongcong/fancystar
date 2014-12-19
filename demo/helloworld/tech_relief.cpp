#include "fscorepch.h"

#include "tech_parallax.h"

using namespace wyc;

class xtech_relief : public xtech_parallax
{
	USE_RTTI;
public:
	xtech_relief() : xtech_parallax()
	{
		m_shader_name = SHADER_RELIEF;
	}
};

REG_RTTI(xtech_relief,xtech_parallax);
