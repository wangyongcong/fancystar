#version 110

uniform sampler2D texmap;
uniform vec4 pickid;

varying vec2 f_texcoord;

void main(void)
{
	if(pickid.a<1.0)
	{
		vec4 c=texture2D(texmap,f_texcoord);
		if(c.a<=pickid.a) 
			discard;
	}
	gl_FragColor=pickid;
}

