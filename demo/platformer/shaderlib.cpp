#include "fscorepch.h"
#include "shaderlib.h"
#include "wyc/render/renderer.h"

const unsigned SHADER_TEXT = wyc::strhash("text");

BEGIN_SHADER(text)
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"gui_proj",
	wyc::USAGE_UNIFORM,"gui_texture",
END_SHADER()

void wyc::xrenderer::initialize_shaders()
{
	USE_SHADER(text,"gui.vs","text.fs",0);
}

