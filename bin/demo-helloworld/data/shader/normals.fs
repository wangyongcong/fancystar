#version 330

in vec3 fs_color;

void main()
{
	gl_FragColor = vec4(fs_color,1.0);
}