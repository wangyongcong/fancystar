#include "fscorepch.h"
#include "wyc/render/renderer.h"

BEGIN_SHADER(simple2d) 
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"mat_camera",
	wyc::USAGE_UNIFORM,"texture",
END_SHADER()

BEGIN_SHADER(glyph_smooth) 
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"mat_camera",
	wyc::USAGE_UNIFORM,"texture",
END_SHADER()

BEGIN_SHADER(glyph_outline) 
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"mat_camera",
	wyc::USAGE_UNIFORM,"texture",
END_SHADER()

BEGIN_SHADER(glyph_glow) 
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"mat_camera",
	wyc::USAGE_UNIFORM,"texture",
END_SHADER()

void wyc::xrenderer::initialize_shaders()
{
	USE_SHADER(simple2d,"simple2d.vs","simple2d.fs",0);
	USE_SHADER(glyph_smooth,"simple2d.vs","glyph_smooth.fs",0);
	USE_SHADER(glyph_outline,"simple2d.vs","glyph_outline.fs",0);
	USE_SHADER(glyph_glow,"simple2d.vs","glyph_glow.fs",0);
}

