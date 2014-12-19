#version 330

const float PI = 3.14159265358979323846264;
const float INV_PI = 1.0/3.14159265358979323846264;

// transform
uniform mat4 mat_camera;
uniform mat4 mat_model;
uniform vec3 scale;

// material
uniform vec3 mtl_diffuse;
uniform vec3 mtl_specular;
uniform float smoothness;

// lighting
uniform vec3 light_ambient;
uniform vec3 light_intensity;
uniform vec3 light_direction;

uniform vec3 eye_position;

in vec3 vertex;
in vec3 normal;

out vec3 f_color;

void main(void) 
{
	mat4 mvp = mat_camera * mat_model;

	gl_Position = mvp * vec4(vertex.xyz,1.0);
	vec3 n = normalize((mat_model * vec4(normal*scale,0)).xyz);
	vec3 l = light_direction;
	float ca = max(dot(n,l),0);

	vec3 v = eye_position - gl_Position.xyz;
	vec3 h = normalize(l+v);
	float ct = max(dot(n,h),0);

	f_color = (INV_PI*mtl_diffuse + (smoothness+8)/(8*PI)*pow(ct,smoothness)*mtl_specular) * light_intensity * ca + light_ambient;
}

