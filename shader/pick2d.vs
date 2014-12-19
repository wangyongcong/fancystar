#version 110

uniform mat4 mvp;
uniform vec2 translate;

attribute vec2 vertex;
attribute vec2 texcoord;

varying vec2 f_texcoord;

void main(void) 
{
	vec4 v=vec4(vertex.x+translate.x,vertex.y+translate.y,0,1.0);
	gl_Position=mvp*v;
	f_texcoord=texcoord;
}

