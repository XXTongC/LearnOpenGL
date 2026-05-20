#version 460 core

const int MAX_POINT_LIGHTS = 16;

in vec2 uv;
out vec4 FragColor;

uniform sampler2D positionRoughnessTexture;
uniform sampler2D normalMetallicTexture;
uniform sampler2D albedoAoTexture;
uniform sampler2D depthTexture;
uniform sampler2D emissiveTexture;

uniform vec3 cameraPosition;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLut;
uniform int useIBL;
uniform float iblDiffuseStrength;
uniform float iblSpecularStrength;
uniform float iblMaxReflectionLod;
uniform float pbrDeferredLightingIntensity;

layout(std430, binding = 3) readonly buffer PBRDeferredLightBuffer
{
	vec4 deferredDirectionalColorIntensity;
	vec4 deferredDirectionalDirectionEnabled;
	vec4 deferredAmbientColor;
	ivec4 deferredPointLightMeta;
	vec4 deferredPointLightColorIntensity[MAX_POINT_LIGHTS];
	vec4 deferredPointLightPositionEnabled[MAX_POINT_LIGHTS];
	vec4 deferredPointLightAttenuation[MAX_POINT_LIGHTS];
};

#include "pbr_lighting.glsl"
#include "pbr_csm_shadow.glsl"
#include "pbr_point_shadow.glsl"

void main()
{
	vec4 positionRoughness = texture(positionRoughnessTexture, uv);
	vec4 normalMetallic = texture(normalMetallicTexture, uv);
	vec4 albedoAo = texture(albedoAoTexture, uv);
	vec3 emissive = texture(emissiveTexture, uv).rgb;
	float depth = texture(depthTexture, uv).r;

	vec3 albedo = albedoAo.rgb;
	vec3 n = normalize(normalMetallic.xyz);
	if (depth >= 1.0 || (length(albedo) <= 0.0001 && length(emissive) <= 0.0001) || length(normalMetallic.xyz) <= 0.0001)
	{
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	vec3 worldPosition = positionRoughness.xyz;
	float roughness = clamp(positionRoughness.a, 0.04, 1.0);
	float metallic = clamp(normalMetallic.a, 0.0, 1.0);
	float ao = clamp(albedoAo.a, 0.0, 1.0);
	vec3 v = normalize(cameraPosition - worldPosition);

	vec3 color = deferredAmbientColor.rgb * albedo * ao;
	if (useIBL == 1)
	{
		color = calculateIblAmbient(n, v, albedo, metallic, roughness, ao);
	}

	float directionalEnabled = deferredDirectionalDirectionEnabled.w;
	vec3 dirLightDirection = normalize(-deferredDirectionalDirectionEnabled.xyz);
	vec3 dirRadiance = deferredDirectionalColorIntensity.rgb * deferredDirectionalColorIntensity.a * directionalEnabled;
	float directionalShadow = calculateCsmShadow(worldPosition, n, dirLightDirection);
	color += calculatePbrLight(dirRadiance, dirLightDirection, n, v, albedo, metallic, roughness) * (1.0 - directionalShadow);

	int pointLightCount = clamp(deferredPointLightMeta.x, 0, MAX_POINT_LIGHTS);
	for (int i = 0; i < pointLightCount; ++i)
	{
		vec3 l = deferredPointLightPositionEnabled[i].xyz - worldPosition;
		float distance = length(l);
		l = normalize(l);
		vec3 attenuationTerms = deferredPointLightAttenuation[i].xyz;
		float attenuation = 1.0 / max(attenuationTerms.x * distance * distance + attenuationTerms.y * distance + attenuationTerms.z, 0.0001);
		vec4 pointColorIntensity = deferredPointLightColorIntensity[i];
		float pointEnabled = deferredPointLightPositionEnabled[i].w;
		float pointShadow = calculatePbrPointShadow(worldPosition, deferredPointLightPositionEnabled[i].xyz, i);
		color += calculatePbrLight(pointColorIntensity.rgb * pointColorIntensity.a * attenuation * pointEnabled, l, n, v, albedo, metallic, roughness) * (1.0 - pointShadow);
	}

	FragColor = vec4(color * max(pbrDeferredLightingIntensity, 0.0) + emissive, 1.0);
}
