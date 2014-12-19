#version 330

uniform sampler2D gui_texture;

in vec4 f_color;
in vec2 f_texcoord;

void main()
{
	gl_FragColor=f_color*texture2D(gui_texture,f_texcoord);
}

