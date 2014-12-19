#version 110

uniform mat4 mvp;
uniform vec3 position;

attribute vec2 vertex;
attribute vec2 tex_base;
attribute vec4 tex_layer0, tex_layer1, tex_layer2, tex_layer3;

varying vec2 f_tex_base;
varying vec4 f_tex_layer[4];

void main(void)
{
	vec4 v=vec4(vertex.xy,0,1.0);
	v.xy+=position.xy;
	gl_Position=mvp*v;
//	gl_Position.z=0.0;
//	gl_Position.w=1.0;
	f_tex_base=tex_base;
	f_tex_layer[0]=tex_layer0;
	f_tex_layer[1]=tex_layer1;
	f_tex_layer[2]=tex_layer2;
	f_tex_layer[3]=tex_layer3;
}

