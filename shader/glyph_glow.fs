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

	vec3 glow_color = vec3(0.0,1.0,0.0);
	vec3 rgb = mix(glow_color, glyph_color, alpha);
	float mu = smoothstep(0.5, 1.1, sqrt(dist));
	out_color = f_color*vec4(rgb, max(alpha,mu));

}

