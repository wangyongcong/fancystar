#version 330

uniform mat4 camera_mvp;

in vec3 position;
in vec2 texcoord;
in vec3 color;

out vec2 f_texcoord;
out vec3 f_color;

void main(void)
{
	gl_Position = camera_mvp*vec4(position,1.0);
	f_texcoord = texcoord;
	f_color = color;
}
