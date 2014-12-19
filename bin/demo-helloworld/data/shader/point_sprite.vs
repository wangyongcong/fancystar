#version 330

uniform mat4 proj2camera;

in vec3 position;

void main()
{
	gl_PointSize = 32.0f;
	gl_Position = proj2camera*vec4(position,1.0);
}
