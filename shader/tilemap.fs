#version 110

uniform sampler2D color_map;

varying vec2 f_tex_base;
varying vec4 f_tex_layer[4];

void main(void)
{
	vec4 src, dst;
	dst=texture2D(color_map,f_tex_base);
	for(int i=0; i<4; ++i) {
		src=texture2D(color_map,f_tex_layer[i].st);
		dst.rgb=dst.rgb*(1.0-src.a)+src.rgb*src.a;
		src=texture2D(color_map,f_tex_layer[i].pq);
		dst.rgb=dst.rgb*(1.0-src.a)+src.rgb*src.a;
	}
	gl_FragColor=dst;
}

