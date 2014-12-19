#version 330

uniform sampler2D gui_texture;

in vec4 f_color;
in vec2 f_texcoord;

out vec4 out_color;

void main()
{
	out_color=vec4(f_color.rgb,f_color.a*texture2D(gui_texture,f_texcoord).a);
//	out_color=vec4(f_color.rgb,f_color.a*texture2D(gui_texture,f_texcoord).a+1.0);
}

