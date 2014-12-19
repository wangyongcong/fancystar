#include "fscorepch.h"
#include "wyc/render/renderer.h"

BEGIN_UNIFORM_BLOCK(gui_context)
	"gui_mvp",
END_UNIFORM_BLOCK()

BEGIN_SHADER(simple2d) 
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"gui_texture",
	wyc::USAGE_UNIFORM_BLOCK,"gui_context",
END_SHADER()

BEGIN_SHADER(glyph) 
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"gui_texture",
	wyc::USAGE_UNIFORM_BLOCK,"gui_context",
END_SHADER()

BEGIN_SHADER(glyph_smooth) 
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"gui_texture",
	wyc::USAGE_UNIFORM_BLOCK,"gui_context",
END_SHADER()

BEGIN_SHADER(glyph_outline) 
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"gui_texture",
	wyc::USAGE_UNIFORM_BLOCK,"gui_context",
END_SHADER()

BEGIN_SHADER(glyph_glow) 
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"gui_texture",
	wyc::USAGE_UNIFORM_BLOCK,"gui_context",
END_SHADER()

void wyc::xrenderer::initialize_shaders()
{
	// register uniform blocks
	REGISTER_UNIFORM_BLOCK(gui_context);

	// load shader programs
	USE_SHADER(simple2d,"simple2d.vs","simple2d.fs",0);
	USE_SHADER(glyph,"simple2d.vs","glyph.fs",0);
	USE_SHADER(glyph_smooth,"simple2d.vs","glyph_smooth.fs",0);
	USE_SHADER(glyph_outline,"simple2d.vs","glyph_outline.fs",0);
	USE_SHADER(glyph_glow,"simple2d.vs","glyph_glow.fs",0);
}

