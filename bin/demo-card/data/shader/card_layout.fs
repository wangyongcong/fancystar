#version 330

uniform sampler2D texture;

in vec2 f_texcoord;

void main()
{
	gl_FragColor=texture2D(texture,f_texcoord);
}

