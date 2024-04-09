#version 460 core
uniform float time;
uniform sampler2D sampler;
in vec3 color;
in vec2 uv;//2
out vec4 FragColor;
void main()
{
	FragColor = texture(sampler, uv);
}