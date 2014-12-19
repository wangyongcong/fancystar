#version 330

uniform mat4 proj;

in vec2 position;
in vec2 texcoord;

out vec2 f_texcoord;

void main (void)
{
	vec4 v=vec4(position.xy,0,1.0);
	gl_Position=proj*v;
	f_texcoord=texcoord;
}
