#version 330

uniform sampler2D base_map;

in vec2 f_texcoord0;

out vec4 frag_color;

void main()
{
	frag_color = texture2D(base_map,f_texcoord0);
}