#version 460 core

const int MAX_POINT_LIGHTS = 4;

struct DirectionalLight
{
	vec3 color;
	vec3 direction;
	float intensity;
};

struct PointLight
{
	vec3 color;
	vec3 position;
	float k2;
	float k1;
	float k0;
};

in vec2 uv;
out vec4 FragColor;

uniform sampler2D positionRoughnessTexture;
uniform sampler2D normalMetallicTexture;
uniform sampler2D albedoAoTexture;
uniform sampler2D depthTexture;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int POINT_LIGHT_NUM;
uniform vec3 ambientColor;
uniform vec3 cameraPosition;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLut;
uniform int useIBL;
uniform float iblDiffuseStrength;
uniform float iblSpecularStrength;
uniform float iblMaxReflectionLod;
uniform float pbrDeferredLightingIntensity;

#include "pbr_lighting.glsl"
#include "pbr_csm_shadow.glsl"

void main()
{
	vec4 positionRoughness = texture(positionRoughnessTexture, uv);
	vec4 normalMetallic = texture(normalMetallicTexture, uv);
	vec4 albedoAo = texture(albedoAoTexture, uv);
	float depth = texture(depthTexture, uv).r;

	vec3 albedo = albedoAo.rgb;
	vec3 n = normalize(normalMetallic.xyz);
	if (depth >= 1.0 || length(albedo) <= 0.0001 || length(normalMetallic.xyz) <= 0.0001)
	{
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	vec3 worldPosition = positionRoughness.xyz;
	float roughness = clamp(positionRoughness.a, 0.04, 1.0);
	float metallic = clamp(normalMetallic.a, 0.0, 1.0);
	float ao = clamp(albedoAo.a, 0.0, 1.0);
	vec3 v = normalize(cameraPosition - worldPosition);

	vec3 color = ambientColor * albedo * ao;
	if (useIBL == 1)
	{
		color = calculateIblAmbient(n, v, albedo, metallic, roughness, ao);
	}

	vec3 dirLightDirection = normalize(-directionalLight.direction);
	vec3 dirRadiance = directionalLight.color * directionalLight.intensity;
	float directionalShadow = calculateCsmShadow(worldPosition, n, dirLightDirection);
	color += calculatePbrLight(dirRadiance, dirLightDirection, n, v, albedo, metallic, roughness) * (1.0 - directionalShadow);

	for (int i = 0; i < POINT_LIGHT_NUM && i < MAX_POINT_LIGHTS; ++i)
	{
		vec3 l = pointLights[i].position - worldPosition;
		float distance = length(l);
		l = normalize(l);
		float attenuation = 1.0 / max(pointLights[i].k2 * distance * distance + pointLights[i].k1 * distance + pointLights[i].k0, 0.0001);
		color += calculatePbrLight(pointLights[i].color * attenuation, l, n, v, albedo, metallic, roughness);
	}

	FragColor = vec4(color * max(pbrDeferredLightingIntensity, 0.0), 1.0);
}
