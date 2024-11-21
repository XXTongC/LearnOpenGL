#version 460 core

uniform float near;
uniform float far;

out vec4 FragColor;
in vec2 uv;
in vec3 normal;

void main()
{
    float Zndc = gl_FragCoord.z * 2.0f - 1.0f;
	float depth = 2.0f * near / (far + near - Zndc * (far - near));
	vec3 finalColor = vec3(depth,depth,depth);
	FragColor = vec4(finalColor ,1.0f );
}
