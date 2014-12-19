#version 330

const float pi = 3.14159265358979323846264;
const float inv_pi = 1.0/3.14159265358979323846264;
// height map in [-1,1]
const float scale = 0.05;
const float bias = -0.03;
// max iterations
const int nstep = 5;
const int nstep_shadow = 5;
const bool enable_shadow = false;

// light properties
uniform vec3 light_intensity;
// material properties
uniform vec3 diffuse;
uniform vec3 specular;
uniform float smoothness;
// global ambient light
const vec3 ambient = vec3(0.1,0.1,0.1);

uniform sampler2D base_map;
uniform sampler2D normal_map;
uniform sampler2D height_map;

// all in tangent space
in vec3 f_lightpos;
in vec3 f_viewpos;
in vec2 f_texcoord;

out vec4 frag_color;

void main (void)
{
	vec3 v = normalize(f_viewpos);
	float hstep = scale + bias;
	float hiter = hstep / nstep;
	vec2 duv = v.xy * hstep / (nstep * v.z);
	vec2 bias_uv = f_texcoord + v.xy * hstep / v.z;
	vec4 height = texture(height_map, bias_uv);
	float hreal = height.r * scale + bias;
	if(hreal<hstep) 
	{
		float hprev;
		// increase steps at oblique view angels
	//	int steps = int(mix(nstep*2, nstep, v.z));
		for(int i=0; i<nstep; ++i)
		{
			bias_uv -= duv;
			hstep -= hiter;
			height = texture(height_map, bias_uv);
			hprev = hreal;
			hreal = height.r * scale + bias;
			if(hreal>=hstep)
			{
				float h1 = (hstep+hiter)-hprev;
				float h2 = hreal-hstep;
				float dt = h1/(h1+h2);
				bias_uv -= duv * dt;
				break;
			}
		}
	}
	// vector component which is between [-1,1] is normalized as RGB color between [0,1]
	// so: normal=(2*color)-1
	vec3 n = texture(normal_map,bias_uv).rgb * 2.0 - vec3(1.0);
	n = normalize(n);
	vec3 l = normalize(f_lightpos);
	vec3 h = normalize(l+v);
	vec3 base_color = texture(base_map,bias_uv).rgb;
	float ca = max(dot(n,l),0);
	vec3 cdiff = inv_pi * diffuse;
	float ct = max(dot(n,h),0);
	vec3 cspec = ((smoothness + 8) / (8 * pi) * pow(ct,smoothness)) * specular;
	
	// self shadowing
	float not_in_shadow = 1.0;
	if(enable_shadow && ca>0) {	
		height = texture(height_map, bias_uv);
		hreal = height.r * scale + bias;
		duv = l.xy * (scale + bias - hreal) / (nstep_shadow * l.z);
		hiter = (scale + bias - hreal) / nstep_shadow;
		hstep = hreal + hiter*0.1;
		for(int i=0; i<nstep_shadow; ++i)
		{	
			bias_uv += duv;
			height = texture(height_map, bias_uv);
			hreal = height.r * scale + bias;
			hstep += hiter;
			if(hreal>hstep)
			{
				not_in_shadow = 0.2;
				break;
			}
		}
	}

	frag_color.rgb = ((cdiff+cspec) * light_intensity * ca * not_in_shadow + ambient) * base_color;
//	frag_color.a = 1.0;
}
