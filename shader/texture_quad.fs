#version 110

uniform sampler2D texmap;

varying vec4 f_color;
varying vec2 f_texcoord;

void main(void)
{
	gl_FragColor=f_color*texture2D(texmap,f_texcoord);
}

