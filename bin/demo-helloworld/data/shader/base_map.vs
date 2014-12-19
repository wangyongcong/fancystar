#version 330

uniform mat4 proj2camera;

in vec3 position;
in vec2 texcoord0;

out vec2 f_texcoord0;

void main()
{
	gl_Position = proj2camera*vec4(position,1.0);
	f_texcoord0 = texcoord0;
}
