#version 330

layout (points) in;
layout (line_strip,max_vertices=2) out;

uniform mat4 proj2camera;
uniform float line_length;
uniform vec3 line_color;

in vec3 gs_normal[];

out vec3 fs_color;

void main()
{
	vec4 pos = proj2camera * gl_in[0].gl_Position;

	gl_Position = pos;
	fs_color = line_color;
	EmitVertex();
	gl_Position = proj2camera * (gl_in[0].gl_Position + vec4(line_length * gs_normal[0],0));
	EmitVertex();
	EndPrimitive();

}