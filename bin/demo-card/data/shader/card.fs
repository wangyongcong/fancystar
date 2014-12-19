#version 330

uniform sampler2D texture;

in vec2 f_texcoord;
in vec3 f_color;

void main(void)
{
	gl_FragColor = texture2D(texture,f_texcoord) * vec4(f_color.xyz,1.0);
	if(gl_FragColor.a<0.1) {
		discard;
	}
}
