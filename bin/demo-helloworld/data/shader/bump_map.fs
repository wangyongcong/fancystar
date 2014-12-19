#version 330

const float pi = 3.14159265358979323846264;
const float inv_pi = 1.0/3.14159265358979323846264;
const vec3 ambient = vec3(0.1,0.1,0.1);

// light properties
uniform vec3 light_intensity;

// material properties
uniform vec3 diffuse;
uniform vec3 specular;
uniform float smoothness;

uniform sampler2D base_map;
uniform sampler2D normal_map;

in vec3 f_lightpos;
in vec3 f_viewpos;
in vec2 f_texcoord;

out vec4 frag_color;

void main (void)
{
	// vector component which is between [-1,1] is normalized as RGB color between [0,1]
	// so: normal=(2*color)-1
	vec3 n = texture(normal_map,f_texcoord).rgb * 2.0 - vec3(1.0);
	n = normalize(n);
	vec3 l = normalize(f_lightpos);
	vec3 h = normalize(f_viewpos);
	h = normalize(l+h);

	float ca = max(dot(n,l),0);
	vec3 cdiff = inv_pi * diffuse;
	float ct = max(dot(n,h),0);
	vec3 cspec = ((smoothness + 8) / (8 * pi) * pow(ct,smoothness)) * specular;
	
	frag_color.rgb = ((cdiff+cspec) * light_intensity  * ca + ambient) * texture(base_map,f_texcoord).rgb;
	frag_color.a = 1.0;
}
