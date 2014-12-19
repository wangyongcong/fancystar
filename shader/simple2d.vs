#version 330

uniform gui_context
{
	mat4 gui_mvp;
};

in vec2 position;
in vec4 color;
in vec2 texcoord;

out vec4 f_color;
out vec2 f_texcoord;

void main (void)
{
	vec4 v=vec4(position.xy,0,1.0);
	gl_Position=v*gui_mvp;
	f_color=color;
	f_texcoord=texcoord;
}
