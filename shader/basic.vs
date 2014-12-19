#version 110

uniform mat4 mvp;

attribute vec4 vertex;
attribute vec4 color;

varying vec4 f_color;

void main(void) 
{
	f_color=color;
	gl_Position=mvp*vertex;
}

