#version 330

uniform mat4 world_transform;
uniform mat3 world_rotation;
uniform mat4 camera_mvp;

in vec4 position;
in vec3 normal;

out vec3 f_normal;
out vec3 f_position;

void main(void)
{
	f_position = (position*world_transform).xyz;
	f_normal = normal*world_rotation;
	gl_Position = position*camera_mvp;
}
