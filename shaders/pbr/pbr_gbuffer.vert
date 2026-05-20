#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aUV;
layout(location = 3) in vec3 aNormal;
layout(location = 4) in vec3 aTangent;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

out vec2 uv;
out vec3 normal;
out vec3 worldPosition;
out mat3 TBN;

void main()
{
	vec4 world = modelMatrix * vec4(aPos, 1.0);
	worldPosition = world.xyz;
	normal = normalize(normalMatrix * normalize(aNormal));
	vec3 tangent = normalize(mat3(modelMatrix) * aTangent);
	vec3 bitangent = normalize(cross(normal, tangent));
	TBN = mat3(tangent, bitangent, normal);
	uv = aUV;
	gl_Position = projectionMatrix * viewMatrix * world;
}
