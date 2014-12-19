#include "fscorepch.h"
#include "shaderlib.h"
#include "wyc/render/renderer.h"

const unsigned SHADER_FLOOR = wyc::strhash("floor");
const unsigned SHADER_PLAYER = wyc::strhash("player");
const unsigned SHADER_CURVE = wyc::strhash("curve");
const unsigned SHADER_GUI = wyc::strhash("gui");
const unsigned SHADER_TEXT = wyc::strhash("text");

BEGIN_SHADER(floor)
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_UNIFORM,"mat_camera",
	wyc::USAGE_UNIFORM,"mat_model",
	wyc::USAGE_UNIFORM,"color",
END_SHADER()

BEGIN_SHADER(player)
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_NORMAL,"normal",
	wyc::USAGE_UNIFORM,"mat_camera",
	wyc::USAGE_UNIFORM,"mat_model",
	wyc::USAGE_UNIFORM,"scale",
	wyc::USAGE_UNIFORM,"mtl_diffuse",
	wyc::USAGE_UNIFORM,"mtl_specular",
	wyc::USAGE_UNIFORM,"smoothness",
	wyc::USAGE_UNIFORM,"eye_position",
	wyc::USAGE_UNIFORM,"light_ambient",
	wyc::USAGE_UNIFORM,"light_intensity",
	wyc::USAGE_UNIFORM,"light_direction",
END_SHADER()

BEGIN_SHADER(curve)
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_UNIFORM,"mat_camera",
	wyc::USAGE_UNIFORM,"position",
	wyc::USAGE_UNIFORM,"color",
END_SHADER()

BEGIN_SHADER(gui)
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"gui_proj",
	wyc::USAGE_UNIFORM,"gui_texture",
END_SHADER()

BEGIN_SHADER(text)
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"gui_proj",
	wyc::USAGE_UNIFORM,"gui_texture",
END_SHADER()

void wyc::xrenderer::initialize_shaders()
{
	USE_SHADER(floor,"floor.vs","floor.fs",0);
	USE_SHADER(player,"player.vs","player.fs",0);
	USE_SHADER(curve,"curve.vs","curve.fs",0);
	USE_SHADER(gui,"gui.vs","gui.fs",0);
	USE_SHADER(text,"gui.vs","text.fs",0);
}

