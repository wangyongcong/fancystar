#include "fscorepch.h"
#include "shaderlib.h"
#include "wyc/render/renderer.h"

const unsigned SHADER_GUI = wyc::strhash("gui");
const unsigned SHADER_TEXT = wyc::strhash("text");
const unsigned SHADER_CARD = wyc::strhash("card");
const unsigned SHADER_CARD_BACK = wyc::strhash("card_back");
const unsigned SHADER_CARD_LAYOUT = wyc::strhash("card_layout");
const unsigned SHADER_LUMA = wyc::strhash("luma");
const unsigned SHADER_FXAA = wyc::strhash("fxaa");

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

BEGIN_SHADER(card) 
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"camera_mvp",
	wyc::USAGE_UNIFORM,"texture",
END_SHADER()

BEGIN_SHADER(fxaa) 
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"screen_sample",
	wyc::USAGE_UNIFORM,"screen_size",
END_SHADER()

BEGIN_SHADER(luma) 
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"screen_sample",
END_SHADER()

BEGIN_SHADER(card_layout)
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"proj",
	wyc::USAGE_UNIFORM,"texture",
END_SHADER()

void wyc::xrenderer::initialize_shaders()
{
	// load shader programs
	USE_SHADER(gui,"gui.vs","gui.fs",0);
	USE_SHADER(text,"gui.vs","text.fs",0);
	USE_SHADER(card,"card.vs","card.fs",0);
	USE_SHADER(fxaa,"fxaa.vs","fxaa.fs",0);
//	USE_SHADER(luma,"fxaa.vs","luma.fs",0);
	USE_SHADER(card_layout,"card_layout.vs","card_layout.fs",0);
}

