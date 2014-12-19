#include "fscorepch.h"
#include "shaderlib.h"
#include "wyc/render/renderer.h"

const unsigned SHADER_TEXT = wyc::strhash("text");
const unsigned SHADER_FLAT_COLOR = wyc::strhash("flat_color");
const unsigned SHADER_BASE_MAP = wyc::strhash("base_map");
const unsigned SHADER_LIGHTING = wyc::strhash("lighting");
const unsigned SHADER_POINT_SPRITE = wyc::strhash("point_sprite");
const unsigned SHADER_BUMP_MAP = wyc::strhash("bump_map");
const unsigned SHADER_NORMALS = wyc::strhash("normals");
const unsigned SHADER_PARALLAX = wyc::strhash("parallax");
const unsigned SHADER_RELIEF = wyc::strhash("relief");

BEGIN_SHADER(text)
	wyc::USAGE_POSITION,"vertex",
	wyc::USAGE_COLOR,"color",
	wyc::USAGE_TEXTURE0,"texcoord",
	wyc::USAGE_UNIFORM,"gui_proj",
	wyc::USAGE_UNIFORM,"gui_texture",
END_SHADER()

BEGIN_SHADER(point_sprite)
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_UNIFORM,"proj2camera",
	wyc::USAGE_UNIFORM,"base_map",
END_SHADER()

BEGIN_SHADER(flat_color)
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_NORMAL,"normal",
	wyc::USAGE_UNIFORM,"local2camera",
	wyc::USAGE_UNIFORM,"proj2camera",
	wyc::USAGE_UNIFORM,"light_position",
	wyc::USAGE_UNIFORM,"light_intensity",
	wyc::USAGE_UNIFORM,"diffuse",
	wyc::USAGE_UNIFORM,"specular",
	wyc::USAGE_UNIFORM,"smoothness",
	wyc::USAGE_UNIFORM,"color",
END_SHADER()

BEGIN_SHADER(base_map)
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_TEXTURE0,"texcoord0",
	wyc::USAGE_UNIFORM,"proj2camera",
	wyc::USAGE_UNIFORM,"base_map",
END_SHADER()

BEGIN_SHADER(lighting)
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_NORMAL,"normal",
	wyc::USAGE_TEXTURE0,"texcoord0",
	wyc::USAGE_UNIFORM,"local2camera",
	wyc::USAGE_UNIFORM,"proj2camera",
	wyc::USAGE_UNIFORM,"light_position",
	wyc::USAGE_UNIFORM,"light_intensity",
	wyc::USAGE_UNIFORM,"diffuse",
	wyc::USAGE_UNIFORM,"specular",
	wyc::USAGE_UNIFORM,"smoothness",
	wyc::USAGE_UNIFORM,"base_map",
END_SHADER()

BEGIN_SHADER(bump_map)
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_NORMAL,"normal",
	wyc::USAGE_TANGENT,"tangent",
	wyc::USAGE_TEXTURE0,"texcoord0",
	wyc::USAGE_UNIFORM,"local2camera",
	wyc::USAGE_UNIFORM,"proj2camera",
	wyc::USAGE_UNIFORM,"light_position",
	wyc::USAGE_UNIFORM,"light_intensity",
	wyc::USAGE_UNIFORM,"diffuse",
	wyc::USAGE_UNIFORM,"specular",
	wyc::USAGE_UNIFORM,"smoothness",
	wyc::USAGE_UNIFORM,"base_map",
	wyc::USAGE_UNIFORM,"normal_map",
END_SHADER()

BEGIN_SHADER(normals)
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_NORMAL,"normal",
	wyc::USAGE_UNIFORM,"proj2camera",
	wyc::USAGE_UNIFORM,"line_length",
	wyc::USAGE_UNIFORM,"line_color",
END_SHADER()

BEGIN_SHADER(parallax)
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_NORMAL,"normal",
	wyc::USAGE_TANGENT,"tangent",
	wyc::USAGE_TEXTURE0,"texcoord0",
	wyc::USAGE_UNIFORM,"local2camera",
	wyc::USAGE_UNIFORM,"proj2camera",
	wyc::USAGE_UNIFORM,"light_position",
	wyc::USAGE_UNIFORM,"light_intensity",
	wyc::USAGE_UNIFORM,"diffuse",
	wyc::USAGE_UNIFORM,"specular",
	wyc::USAGE_UNIFORM,"smoothness",
	wyc::USAGE_UNIFORM,"base_map",
	wyc::USAGE_UNIFORM,"normal_map",
	wyc::USAGE_UNIFORM,"height_map",
END_SHADER()

BEGIN_SHADER(relief)
	wyc::USAGE_POSITION,"position",
	wyc::USAGE_NORMAL,"normal",
	wyc::USAGE_TANGENT,"tangent",
	wyc::USAGE_TEXTURE0,"texcoord0",
	wyc::USAGE_UNIFORM,"local2camera",
	wyc::USAGE_UNIFORM,"proj2camera",
	wyc::USAGE_UNIFORM,"light_position",
	wyc::USAGE_UNIFORM,"light_intensity",
	wyc::USAGE_UNIFORM,"diffuse",
	wyc::USAGE_UNIFORM,"specular",
	wyc::USAGE_UNIFORM,"smoothness",
	wyc::USAGE_UNIFORM,"base_map",
	wyc::USAGE_UNIFORM,"normal_map",
	wyc::USAGE_UNIFORM,"height_map",
END_SHADER()

void wyc::xrenderer::initialize_shaders()
{
	USE_SHADER(text,"gui.vs","text.fs",0);
	USE_SHADER(point_sprite,"point_sprite.vs","point_sprite.fs",0);
	USE_SHADER(flat_color,"light_color.vs","light_color.fs",0);
	USE_SHADER(base_map,"base_map.vs","base_map.fs",0);
	USE_SHADER(lighting,"lighting.vs","lighting.fs",0);
	USE_SHADER(bump_map,"bump_map.vs","bump_map.fs",0);
	USE_SHADER(normals,"normals.vs","normals.fs","normals.gs");
	USE_SHADER(parallax,"parallax.vs","parallax.fs",0);
	USE_SHADER(relief,"parallax.vs","relief.fs",0);
}

