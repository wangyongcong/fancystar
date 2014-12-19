#include "fscorepch.h"
#include "renderer.h"

BEGIN_SHADER(simple)
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_UNIFORM,"mvp",
	wyc::USAGE_UNIFORM,"color",
END_SHADER()

BEGIN_SHADER(draw2d)
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"mvp",
	wyc::USAGE_UNIFORM,"translate",
	wyc::USAGE_UNIFORM,"texmap",
END_SHADER()

BEGIN_SHADER(pick2d)
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"mvp",
	wyc::USAGE_UNIFORM,"translate",
	wyc::USAGE_UNIFORM,"texmap",
	wyc::USAGE_UNIFORM,"pickid",
END_SHADER()

void wyc::xrenderer::initialize_shaders()
{
	USE_SHADER(simple);
	USE_SHADER(draw2d);
	USE_SHADER(pick2d);
}

