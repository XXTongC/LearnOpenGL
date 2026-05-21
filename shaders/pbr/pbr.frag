#version 460 core

const int MAX_POINT_LIGHTS = 4;

struct DirectionalLight
{
	vec3 color;
	vec3 direction;
	float specularIntensity;
	float intensity;
};

struct PointLight
{
	vec3 color;
	vec3 position;
	float specularIntensity;
	float k2;
	float k1;
	float k0;
};

in vec2 uv;
in vec3 normal;
in vec3 worldPosition;
in mat3 TBN;

out vec4 FragColor;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int POINT_LIGHT_NUM;

uniform vec3 ambientColor;
uniform vec3 cameraPosition;
uniform float opacity;

uniform vec3 pbrAlbedo;
uniform float pbrMetallic;
uniform float pbrRoughness;
uniform float pbrAo;
uniform vec3 pbrEmissiveColor;
uniform float pbrEmissiveIntensity;
uniform int useAlphaMask;
uniform float alphaCutoff;

uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D emissiveMap;
uniform sampler2D normalMap;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLut;
uniform int useAlbedoMap;
uniform int useMetallicMap;
uniform int useRoughnessMap;
uniform int useAoMap;
uniform int useEmissiveMap;
uniform int useNormalMap;
uniform int useIBL;
uniform float iblDiffuseStrength;
uniform float iblSpecularStrength;
uniform float iblMaxReflectionLod;

#include "pbr_lighting.glsl"
#include "pbr_csm_shadow.glsl"
#include "pbr_point_shadow.glsl"

void main()
{
	vec3 n = normalize(normal);
	if (useNormalMap == 1)
	{
		vec3 tangentNormal = texture(normalMap, uv).rgb * 2.0 - vec3(1.0);
		n = normalize(TBN * tangentNormal);
	}

	vec3 v = normalize(cameraPosition - worldPosition);

	vec3 albedo = pbrAlbedo;
	float albedoAlpha = 1.0;
	if (useAlbedoMap == 1)
	{
		vec4 albedoSample = texture(albedoMap, uv);
		albedo = pow(albedoSample.rgb, vec3(2.2));
		albedoAlpha = albedoSample.a;
	}
	if (useAlphaMask == 1 && albedoAlpha < alphaCutoff)
	{
		discard;
	}

	float metallic = clamp(pbrMetallic, 0.0, 1.0);
	if (useMetallicMap == 1)
	{
		metallic = texture(metallicMap, uv).r;
	}

	float roughness = clamp(pbrRoughness, 0.04, 1.0);
	if (useRoughnessMap == 1)
	{
		roughness = clamp(texture(roughnessMap, uv).r, 0.04, 1.0);
	}

	float ao = clamp(pbrAo, 0.0, 1.0);
	if (useAoMap == 1)
	{
		ao = texture(aoMap, uv).r;
	}

	vec3 lightColor = directionalLight.color * directionalLight.intensity;
	vec3 lightDirection = normalize(-directionalLight.direction);
	float directionalShadow = calculateCsmShadow(worldPosition, n, lightDirection);
	vec3 color = calculatePbrLight(lightColor, lightDirection, n, v, albedo, metallic, roughness) * (1.0 - directionalShadow);

	for (int i = 0; i < POINT_LIGHT_NUM && i < MAX_POINT_LIGHTS; ++i)
	{
		vec3 l = pointLights[i].position - worldPosition;
		float distance = length(l);
		l = normalize(l);
		float attenuation = 1.0 / max(pointLights[i].k2 * distance * distance + pointLights[i].k1 * distance + pointLights[i].k0, 0.0001);
		float pointShadow = calculatePbrPointShadow(worldPosition, pointLights[i].position, i);
		color += calculatePbrLight(pointLights[i].color * attenuation, l, n, v, albedo, metallic, roughness) * (1.0 - pointShadow);
	}

	vec3 emissive = pbrEmissiveColor * pbrEmissiveIntensity;
	if (useEmissiveMap == 1)
	{
		emissive += texture(emissiveMap, uv).rgb * pbrEmissiveIntensity;
	}

	vec3 ambient = ambientColor * albedo * ao;
	if (useIBL == 1)
	{
		ambient = calculateIblAmbient(n, v, albedo, metallic, roughness, ao);
	}

	color += ambient + emissive;
	FragColor = vec4(color, opacity);
}
