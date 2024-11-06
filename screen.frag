#version 460 core
out vec4 FragColor;
in vec2 uv;

uniform sampler2D screenTextureSampler;

void main()
{
    FragColor = texture(screenTextureSampler,uv);
}