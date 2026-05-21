#version 460 core

in vec2 uv;
in vec3 normal;
in vec3 worldPosition;
in mat3 TBN;

layout(location = 0) out vec4 gPositionRoughness;
layout(location = 1) out vec4 gNormalMetallic;
layout(location = 2) out vec4 gAlbedoAo;
layout(location = 3) out vec4 gEmissive;
layout(location = 4) out vec4 gMaterialParams;

uniform vec3 pbrAlbedo;
uniform float pbrMetallic;
uniform float pbrRoughness;
uniform float pbrAo;
uniform vec3 pbrEmissiveColor;
uniform float pbrEmissiveIntensity;
uniform int useAlphaMask;
uniform float alphaCutoff;
uniform int useIBL;
uniform float iblDiffuseStrength;
uniform float iblSpecularStrength;

uniform sampler2D albedoMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D emissiveMap;
uniform sampler2D normalMap;
uniform int useAlbedoMap;
uniform int useMetallicMap;
uniform int useRoughnessMap;
uniform int useAoMap;
uniform int useEmissiveMap;
uniform int useNormalMap;

void main()
{
	vec3 n = normalize(normal);
	if (useNormalMap == 1)
	{
		vec3 tangentNormal = texture(normalMap, uv).rgb * 2.0 - vec3(1.0);
		n = normalize(TBN * tangentNormal);
	}

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

	vec3 emissive = pbrEmissiveColor * pbrEmissiveIntensity;
	if (useEmissiveMap == 1)
	{
		emissive += texture(emissiveMap, uv).rgb * pbrEmissiveIntensity;
	}

	gPositionRoughness = vec4(worldPosition, roughness);
	gNormalMetallic = vec4(normalize(n), metallic);
	gAlbedoAo = vec4(albedo, ao);
	gEmissive = vec4(emissive, 1.0);
	gMaterialParams = vec4(useIBL == 1 ? 1.0 : 0.0, iblDiffuseStrength, iblSpecularStrength, 0.0);
}
