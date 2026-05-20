#version 460 core

in vec2 uv;
out vec4 FragColor;

uniform samplerCube environmentMap;
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLut;
uniform int iblDebugMode;
uniform float iblDebugMipLevel;
uniform float iblDebugIntensity;

vec3 directionFromUv(vec2 value)
{
	vec2 p = value * 2.0 - 1.0;
	return normalize(vec3(p.x, -p.y, 1.0));
}

void main()
{
	vec3 direction = directionFromUv(uv);
	vec3 color = vec3(0.0);

	if (iblDebugMode == 1)
	{
		color = texture(irradianceMap, direction).rgb;
	}
	else if (iblDebugMode == 2)
	{
		color = textureLod(prefilterMap, direction, max(iblDebugMipLevel, 0.0)).rgb;
	}
	else if (iblDebugMode == 3)
	{
		vec2 brdf = texture(brdfLut, uv).rg;
		color = vec3(brdf, 0.0);
	}
	else
	{
		color = texture(environmentMap, direction).rgb;
	}

	FragColor = vec4(color * max(iblDebugIntensity, 0.0), 1.0);
}
