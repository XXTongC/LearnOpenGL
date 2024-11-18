#version 460 core
out vec4 FragColor;
in vec2 uv;

uniform sampler2D screenTextureSampler;

vec3 colorInvert(vec3 color)
{
    vec3 invertColor = vec3(1.0f) - color;
    return invertColor;
}

void main()
{
    vec3 color = texture(screenTextureSampler,uv).rgb;

    FragColor = vec4(color, 1.0f);
}