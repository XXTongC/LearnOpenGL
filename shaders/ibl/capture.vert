#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 localPosition;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;

void main()
{
	localPosition = aPos;
	gl_Position = projectionMatrix * viewMatrix * vec4(aPos, 1.0);
}
