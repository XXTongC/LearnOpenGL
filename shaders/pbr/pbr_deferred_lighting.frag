#version 460 core

const int MAX_POINT_LIGHTS = 16;

in vec2 uv;
out vec4 FragColor;

uniform sampler2D positionRoughnessTexture;
uniform sampler2D normalMetallicTexture;
uniform sampler2D albedoAoTexture;
uniform sampler2D depthTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D materialParamsTexture;

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

layout(std430, binding = 4) readonly buffer PBRDeferredTileBuffer
{
	ivec4 deferredTileOffsetCount[];
};

layout(std430, binding = 5) readonly buffer PBRDeferredTileIndexBuffer
{
	int deferredTileLightIndices[];
};

layout(std430, binding = 6) readonly buffer PBRDeferredClusterBuffer
{
	ivec4 deferredClusterOffsetCount[];
};

layout(std430, binding = 7) readonly buffer PBRDeferredClusterIndexBuffer
{
	int deferredClusterLightIndices[];
};

uniform int useClusteredPointLights;
uniform int clusteredLightTileSize;
uniform int clusteredLightGridColumns;
uniform int clusteredLightGridRows;
uniform int clusteredLightDepthSlices;
uniform float clusteredLightNearPlane;
uniform float clusteredLightFarPlane;
uniform int useTiledPointLights;
uniform int tiledLightTileSize;
uniform int tiledLightGridColumns;
uniform int tiledLightGridRows;

#include "pbr_lighting.glsl"
#include "pbr_csm_shadow.glsl"
#include "pbr_point_shadow.glsl"

vec3 calculateDeferredPointLight(
	int lightIndex,
	vec3 worldPosition,
	vec3 n,
	vec3 v,
	vec3 albedo,
	float metallic,
	float roughness
)
{
	vec3 l = deferredPointLightPositionEnabled[lightIndex].xyz - worldPosition;
	float distance = length(l);
	l = normalize(l);
	vec3 attenuationTerms = deferredPointLightAttenuation[lightIndex].xyz;
	float attenuation = 1.0 / max(attenuationTerms.x * distance * distance + attenuationTerms.y * distance + attenuationTerms.z, 0.0001);
	vec4 pointColorIntensity = deferredPointLightColorIntensity[lightIndex];
	float pointEnabled = deferredPointLightPositionEnabled[lightIndex].w;
	float pointShadow = calculatePbrPointShadow(worldPosition, deferredPointLightPositionEnabled[lightIndex].xyz, lightIndex);
	return calculatePbrLight(pointColorIntensity.rgb * pointColorIntensity.a * attenuation * pointEnabled, l, n, v, albedo, metallic, roughness) * (1.0 - pointShadow);
}

void main()
{
	vec4 positionRoughness = texture(positionRoughnessTexture, uv);
	vec4 normalMetallic = texture(normalMetallicTexture, uv);
	vec4 albedoAo = texture(albedoAoTexture, uv);
	vec3 emissive = texture(emissiveTexture, uv).rgb;
	vec4 materialParams = texture(materialParamsTexture, uv);
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
	if (useIBL == 1 && materialParams.r > 0.5)
	{
		color = calculateIblAmbientWithStrength(
			n,
			v,
			albedo,
			metallic,
			roughness,
			ao,
			materialParams.g,
			materialParams.b
		);
	}

	float directionalEnabled = deferredDirectionalDirectionEnabled.w;
	vec3 dirLightDirection = normalize(-deferredDirectionalDirectionEnabled.xyz);
	vec3 dirRadiance = deferredDirectionalColorIntensity.rgb * deferredDirectionalColorIntensity.a * directionalEnabled;
	float directionalShadow = calculateCsmShadow(worldPosition, n, dirLightDirection);
	color += calculatePbrLight(dirRadiance, dirLightDirection, n, v, albedo, metallic, roughness) * (1.0 - directionalShadow);

	int pointLightCount = clamp(deferredPointLightMeta.x, 0, MAX_POINT_LIGHTS);
	if (useClusteredPointLights == 1 && clusteredLightGridColumns > 0 && clusteredLightGridRows > 0 && clusteredLightDepthSlices > 0 && clusteredLightTileSize > 0)
	{
		ivec2 tileCoord = ivec2(gl_FragCoord.xy) / clusteredLightTileSize;
		tileCoord = clamp(tileCoord, ivec2(0), ivec2(clusteredLightGridColumns - 1, clusteredLightGridRows - 1));
		float viewDepth = -(viewMatrix * vec4(worldPosition, 1.0)).z;
		float depthRange = max(clusteredLightFarPlane - clusteredLightNearPlane, 0.001);
		int depthSlice = int(floor(((viewDepth - clusteredLightNearPlane) / depthRange) * float(clusteredLightDepthSlices)));
		depthSlice = clamp(depthSlice, 0, clusteredLightDepthSlices - 1);
		int clusterIndex = (depthSlice * clusteredLightGridRows + tileCoord.y) * clusteredLightGridColumns + tileCoord.x;
		ivec4 offsetCount = deferredClusterOffsetCount[clusterIndex];
		int offset = max(offsetCount.x, 0);
		int count = max(offsetCount.y, 0);
		for (int entry = 0; entry < count; ++entry)
		{
			int lightIndex = deferredClusterLightIndices[offset + entry];
			if (lightIndex >= 0 && lightIndex < pointLightCount)
			{
				color += calculateDeferredPointLight(lightIndex, worldPosition, n, v, albedo, metallic, roughness);
			}
		}
	}
	else if (useTiledPointLights == 1 && tiledLightGridColumns > 0 && tiledLightGridRows > 0 && tiledLightTileSize > 0)
	{
		ivec2 tileCoord = ivec2(gl_FragCoord.xy) / tiledLightTileSize;
		tileCoord = clamp(tileCoord, ivec2(0), ivec2(tiledLightGridColumns - 1, tiledLightGridRows - 1));
		int tileIndex = tileCoord.y * tiledLightGridColumns + tileCoord.x;
		ivec4 offsetCount = deferredTileOffsetCount[tileIndex];
		int offset = max(offsetCount.x, 0);
		int count = max(offsetCount.y, 0);
		for (int entry = 0; entry < count; ++entry)
		{
			int lightIndex = deferredTileLightIndices[offset + entry];
			if (lightIndex >= 0 && lightIndex < pointLightCount)
			{
				color += calculateDeferredPointLight(lightIndex, worldPosition, n, v, albedo, metallic, roughness);
			}
		}
	}
	else
	{
		for (int i = 0; i < pointLightCount; ++i)
		{
			color += calculateDeferredPointLight(i, worldPosition, n, v, albedo, metallic, roughness);
		}
	}

	FragColor = vec4(color * max(pbrDeferredLightingIntensity, 0.0) + emissive, 1.0);
}
