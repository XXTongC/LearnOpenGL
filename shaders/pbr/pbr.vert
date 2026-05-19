#version 460 core

in vec3 aPos;
in vec4 aColor;
in vec2 aUV;
in vec3 aNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

out vec2 uv;
out vec4 color;
out vec3 normal;
out vec3 worldPosition;

void main()
{
	vec4 world = modelMatrix * vec4(aPos, 1.0);
	worldPosition = world.xyz;
	normal = normalize(normalMatrix * normalize(aNormal));
	uv = aUV;
	color = aColor;
	gl_Position = projectionMatrix * viewMatrix * world;
}
