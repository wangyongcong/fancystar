#version 110

uniform mat4 mat_camera;
uniform vec2 translate;

attribute vec2 vertex;
attribute vec4 color;
attribute vec2 texcoord;

varying vec4 f_color;
varying vec2 f_texcoord;

void main(void)
{
	vec4 v=vec4(vertex.x+translate.x,vertex.y+translate.y,0,1.0);
	gl_Position=v*mat_camera;
	f_color=color;
	f_texcoord=texcoord;
}

