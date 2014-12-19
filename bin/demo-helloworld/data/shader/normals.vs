#version 330

in vec3 position;
in vec3 normal;

out vec3 gs_normal;

void main()
{
	gl_Position = vec4(position,1.0);
	gs_normal = normal;
}
