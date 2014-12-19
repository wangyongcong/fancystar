#version 110

uniform mat4 mvp;
uniform vec2 translate;

attribute vec2 vertex;
attribute vec4 color;
attribute vec2 texcoord;

varying vec4 f_color;
varying vec2 f_texcoord;

void main(void)
{
	vec4 v=vec4(vertex.x+translate.x,vertex.y+translate.y,0,1.0);
	gl_Position=mvp*v;
	f_color=color;
	f_texcoord=texcoord;
}

