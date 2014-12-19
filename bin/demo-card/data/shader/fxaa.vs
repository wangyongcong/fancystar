#version 330

in vec3 position;
in vec2 texcoord;

out vec2 f_texcoord;

void main(void)
{
	gl_Position = vec4(position.xyz,1.0);
	f_texcoord = texcoord;
}
