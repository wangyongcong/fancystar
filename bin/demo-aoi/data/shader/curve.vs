#version 110

uniform mat4 mat_camera;
uniform vec3 position;

attribute vec2 vertex;

void main(void) 
{
	vec4 v = vec4(vertex.x,0,vertex.y,1.0);
	v.xyz += position;
	gl_Position = mat_camera*v;
}

