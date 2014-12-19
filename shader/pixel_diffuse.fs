#version 330

// light properties
uniform vec3 ambient;
uniform vec3 eye_position;
uniform vec3 light_position[2];
uniform vec3 light_intensity[2];
uniform uint light_count=1u;

// material properties
uniform vec3 mtl_diffuse;
uniform vec3 mtl_specular;
uniform float mtl_smoothness;

in vec3 f_position;
in vec3 f_normal;

const float pi = 3.14159265358979323846264;
const float inv_pi = 1.0/3.14159265358979323846264;

vec3 lighting_pass (vec3 position, vec3 intensity)
{
	vec3 n = normalize(f_normal);
	vec3 l = position - f_position;
	l = normalize(l);
	float ca = max(dot(n,l),0);
	vec3 cdiff = inv_pi * mtl_diffuse; 

	vec3 v = eye_position - f_position;
	vec3 h = l+v;
	h = normalize(h);
	float ct = max(dot(n,h),0);
	vec3 cspec = ((mtl_smoothness + 8) / (8 * pi) * pow(ct,mtl_smoothness)) * mtl_specular;
	
	return (cdiff+cspec) * intensity *ca;
}

void main(void)
{	
	vec3 c = ambient;
	for(uint i=0u; i<light_count; i+=1u)
		c += lighting_pass(light_position[i], light_intensity[i]);
	gl_FragColor.rgb=c;
	gl_FragColor.a=1.0;
}
