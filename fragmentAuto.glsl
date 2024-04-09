#version 460 core
uniform float time;
out vec4 FragColor;
in vec3 color;
in vec3 position;
void main()
{
	FragColor = vec4(color,1.0f);
}