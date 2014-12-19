#version 330

uniform mat4 mat_camera;
uniform mat4 mat_model;

in vec2 vertex;

void main(void) 
{
	mat4 mvp = mat_camera * mat_model;
	gl_Position = mvp * vec4(vertex.x,0,vertex.y,1.0);
}

