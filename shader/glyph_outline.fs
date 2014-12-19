#version 330

uniform sampler2D gui_texture;

in vec4 f_color;
in vec2 f_texcoord;

const vec3 glyph_color = vec3(1.0,1.0,1.0);

out vec4 out_color;

void main()
{
    float dist  = texture2D(gui_texture, f_texcoord).a;
    float width = fwidth(dist);
    float alpha = smoothstep(0.5-width, 0.5+width, dist);
	
	// outline
	vec3 outline_color = vec3(1.0,0.0,0.0);
	float mu = smoothstep(0.52-width, 0.52+width, dist);
	vec3 rgb = mix(outline_color, glyph_color, mu);
	out_color = f_color*vec4(rgb, max(alpha,mu));
	out_color = vec4(rgb, max(alpha,mu));
}

