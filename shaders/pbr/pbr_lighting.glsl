const float PBR_PI = 3.14159265359;

float distributionGGX(vec3 n, vec3 h, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float nDotH = max(dot(n, h), 0.0);
	float nDotH2 = nDotH * nDotH;
	float denominator = (nDotH2 * (a2 - 1.0) + 1.0);
	denominator = PBR_PI * denominator * denominator;
	return a2 / max(denominator, 0.0001);
}

float geometrySchlickGGX(float nDotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0;
	return nDotV / max(nDotV * (1.0 - k) + k, 0.0001);
}

float geometrySmith(vec3 n, vec3 v, vec3 l, float roughness)
{
	float nDotV = max(dot(n, v), 0.0);
	float nDotL = max(dot(n, l), 0.0);
	return geometrySchlickGGX(nDotV, roughness) * geometrySchlickGGX(nDotL, roughness);
}

vec3 fresnelSchlick(float cosTheta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness)
{
	return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 calculatePbrLight(vec3 radiance, vec3 l, vec3 n, vec3 v, vec3 albedo, float metallic, float roughness)
{
	vec3 h = normalize(v + l);
	float nDotL = max(dot(n, l), 0.0);
	float nDotV = max(dot(n, v), 0.0);
	vec3 f0 = mix(vec3(0.04), albedo, metallic);

	float ndf = distributionGGX(n, h, roughness);
	float geometry = geometrySmith(n, v, l, roughness);
	vec3 fresnel = fresnelSchlick(max(dot(h, v), 0.0), f0);

	vec3 numerator = ndf * geometry * fresnel;
	float denominator = 4.0 * nDotV * nDotL + 0.0001;
	vec3 specular = numerator / denominator;

	vec3 kS = fresnel;
	vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
	return (kD * albedo / PBR_PI + specular) * radiance * nDotL;
}

vec3 calculateIblAmbient(vec3 n, vec3 v, vec3 albedo, float metallic, float roughness, float ao)
{
	vec3 f0 = mix(vec3(0.04), albedo, metallic);
	float nDotV = max(dot(n, v), 0.0);
	vec3 fresnel = fresnelSchlickRoughness(nDotV, f0, roughness);
	vec3 kS = fresnel;
	vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

	vec3 irradiance = texture(irradianceMap, n).rgb;
	vec3 diffuse = irradiance * albedo;

	vec3 reflection = reflect(-v, n);
	vec3 prefilteredColor = textureLod(prefilterMap, reflection, roughness * iblMaxReflectionLod).rgb;
	vec2 brdf = texture(brdfLut, vec2(nDotV, roughness)).rg;
	vec3 specular = prefilteredColor * (fresnel * brdf.x + brdf.y);

	return (kD * diffuse * iblDiffuseStrength + specular * iblSpecularStrength) * ao;
}
