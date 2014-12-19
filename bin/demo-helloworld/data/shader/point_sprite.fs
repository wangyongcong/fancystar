#version 330

uniform sampler2D base_map;

out vec4 frag_color;

void main()
{
	frag_color = texture(base_map,gl_PointCoord);
}