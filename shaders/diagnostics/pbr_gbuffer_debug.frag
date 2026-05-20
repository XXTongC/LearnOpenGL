#version 460 core

in vec2 uv;
out vec4 FragColor;

uniform sampler2D positionRoughnessTexture;
uniform sampler2D normalMetallicTexture;
uniform sampler2D albedoAoTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D materialParamsTexture;
uniform sampler2D depthTexture;
uniform int gbufferDebugMode;
uniform float gbufferDebugIntensity;

vec3 visualizeWorldPosition(vec3 position)
{
	return fract(abs(position) * 0.25);
}

void main()
{
	vec4 positionRoughness = texture(positionRoughnessTexture, uv);
	vec4 normalMetallic = texture(normalMetallicTexture, uv);
	vec4 albedoAo = texture(albedoAoTexture, uv);
	vec3 emissive = texture(emissiveTexture, uv).rgb;
	vec4 materialParams = texture(materialParamsTexture, uv);
	float depth = texture(depthTexture, uv).r;

	vec3 color = albedoAo.rgb;
	if (gbufferDebugMode == 1)
	{
		color = normalize(normalMetallic.xyz) * 0.5 + vec3(0.5);
	}
	else if (gbufferDebugMode == 2)
	{
		color = vec3(positionRoughness.a);
	}
	else if (gbufferDebugMode == 3)
	{
		color = vec3(normalMetallic.a);
	}
	else if (gbufferDebugMode == 4)
	{
		color = vec3(albedoAo.a);
	}
	else if (gbufferDebugMode == 5)
	{
		color = vec3(depth);
	}
	else if (gbufferDebugMode == 6)
	{
		color = visualizeWorldPosition(positionRoughness.xyz);
	}
	else if (gbufferDebugMode == 7)
	{
		color = emissive;
	}
	else if (gbufferDebugMode == 8)
	{
		color = vec3(materialParams.g, materialParams.b, materialParams.r);
	}

	FragColor = vec4(color * max(gbufferDebugIntensity, 0.0), 1.0);
}
