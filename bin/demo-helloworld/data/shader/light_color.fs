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
uniform vec3 color;

in vec3 f_normal;
in vec3 f_lightpos;
in vec3 f_viewpos;

out vec4 frag_color;

void main (void)
{
	vec3 n = normalize(f_normal);
	vec3 l = normalize(f_lightpos);
	vec3 h = normalize(f_viewpos);
	h = normalize(h+l);

	float ca = max(dot(n,l),0);
	vec3 cdiff = inv_pi * diffuse;
	float ct = max(dot(n,h),0);
	vec3 cspec = ((smoothness + 8) / (8 * pi) * pow(ct,smoothness)) * specular;
	
	frag_color.rgb = ((cdiff+cspec) * light_intensity * ca + ambient) * color;
	frag_color.a = 1.0;
}
