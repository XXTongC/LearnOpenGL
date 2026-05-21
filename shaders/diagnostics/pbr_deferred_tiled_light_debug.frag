#version 460 core

in vec2 uv;
out vec4 FragColor;

layout(std430, binding = 4) readonly buffer PBRDeferredTileBuffer
{
	ivec4 deferredTileOffsetCount[];
};

uniform int tiledLightTileSize;
uniform int tiledLightGridColumns;
uniform int tiledLightGridRows;
uniform int tiledLightDebugMaxLights;
uniform float tiledLightDebugIntensity;

vec3 heatmap(float value)
{
	value = clamp(value, 0.0, 1.0);
	vec3 cold = vec3(0.02, 0.05, 0.16);
	vec3 low = vec3(0.0, 0.55, 1.0);
	vec3 mid = vec3(1.0, 0.85, 0.1);
	vec3 high = vec3(1.0, 0.05, 0.02);
	if (value < 0.5)
	{
		return mix(cold, low, value * 2.0);
	}
	return mix(mid, high, (value - 0.5) * 2.0);
}

void main()
{
	if (tiledLightGridColumns <= 0 || tiledLightGridRows <= 0 || tiledLightTileSize <= 0)
	{
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	ivec2 tileCoord = ivec2(gl_FragCoord.xy) / tiledLightTileSize;
	tileCoord = clamp(tileCoord, ivec2(0), ivec2(tiledLightGridColumns - 1, tiledLightGridRows - 1));
	int tileIndex = tileCoord.y * tiledLightGridColumns + tileCoord.x;
	int lightCount = max(deferredTileOffsetCount[tileIndex].y, 0);
	float normalizedCount = float(lightCount) / float(max(tiledLightDebugMaxLights, 1));

	vec3 color = heatmap(normalizedCount) * max(tiledLightDebugIntensity, 0.0);
	FragColor = vec4(color, 1.0);
}
