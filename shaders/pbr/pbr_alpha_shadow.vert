#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aUV;

uniform mat4 lightMatrix;
uniform mat4 modelMatrix;

out vec2 uv;

void main()
{
	uv = aUV;
	gl_Position = lightMatrix * modelMatrix * vec4(aPos, 1.0);
}
