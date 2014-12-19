#include "fscorepch.h"
#include "renderer.h"

BEGIN_SHADER(simple)
	wyc::USAGE_POSITION,"vertex",
END_SHADER()

BEGIN_SHADER(basic) 
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_COLOR,"color",
END_SHADER()

BEGIN_SHADER(draw2d)
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_TEXTURE0,"tex_coord",
END_SHADER()

BEGIN_SHADER(tilemap)
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_COLOR,"tex_base",
	wyc::USAGE_TEXTURE0, "tex_layer0",
	wyc::USAGE_TEXTURE1, "tex_layer1",
	wyc::USAGE_TEXTURE2, "tex_layer2",
	wyc::USAGE_TEXTURE3, "tex_layer3",
END_SHADER()

void wyc::xrenderer::initialize_shaders()
{
	USE_SHADER(simple);
	USE_SHADER(tilemap);
}

