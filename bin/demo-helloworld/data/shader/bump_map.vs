#version 330

uniform mat4 local2camera;
uniform mat4 proj2camera;
uniform vec3 light_position; // in camera space

in vec4 position;
in vec3 normal;
in vec4 tangent;
in vec2 texcoord0;

out vec3 f_lightpos;
out vec3 f_viewpos;
out vec2 f_texcoord;

void main(void)
{
	vec3 N = (local2camera*vec4(normal,0)).xyz;
	N = normalize(N);
	vec3 T = (local2camera*vec4(tangent.xyz,0)).xyz;
	T = normalize(T);
	// if N and T are normalized and orthogonal
	// the cross product of them is normalized, too
	vec3 B =  cross(N,T) * tangent.w;
	mat3 camera2tangent = transpose(mat3(T,B,N));

	f_viewpos = -(local2camera*position).xyz;
	f_lightpos = camera2tangent*(light_position + f_viewpos);
	f_viewpos = camera2tangent*f_viewpos;
	f_texcoord = texcoord0;
	gl_Position = proj2camera*position;
}
