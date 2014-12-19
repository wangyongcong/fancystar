#version 110

uniform mat4 mat_camera;
uniform mat4 mat_local2world;
attribute vec4 vertex;

void main(void) 
{
	mat4 mvp=mat_local2world*mat_camera;
	gl_Position=vertex*mvp;
}

