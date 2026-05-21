#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aUV;

uniform mat4 modelMatrix;
uniform mat4 lightSpaceMatrix;

out vec2 uv;
out vec3 worldPosition;

void main()
{
	uv = aUV;
	worldPosition = (modelMatrix * vec4(aPos, 1.0)).xyz;
	gl_Position = lightSpaceMatrix * modelMatrix * vec4(aPos, 1.0);
}
