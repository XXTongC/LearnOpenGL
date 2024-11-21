#version 460 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in vec3 aPos; //0

in vec2 aUV;//2
in vec3 aNormal;//3

out vec2 uv;
out vec3 normal;

void main()
{	

	vec4 transformPosition = vec4(aPos,1.0);

	transformPosition = modelMatrix * transformPosition;

	gl_Position = projectionMatrix * viewMatrix * transformPosition;

	uv = aUV;
	normal = normalize(aNormal);
}