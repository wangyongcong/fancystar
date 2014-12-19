#version 330

uniform mat4 local2camera;
uniform mat4 proj2camera;
uniform vec3 light_position; // camera space

in vec4 position;
in vec3 normal;
in vec2 texcoord0;

out vec3 f_normal;
out vec2 f_texcoord;
out vec3 f_lightpos;
out vec3 f_viewpos;

void main(void)
{
	f_viewpos = -(local2camera*position).xyz;
	f_lightpos = light_position + f_viewpos;
	f_normal = (local2camera*vec4(normal,0)).xyz;

	f_texcoord = texcoord0;
	gl_Position = proj2camera*position;
}
