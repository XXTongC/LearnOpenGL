#version 460 core

in vec2 uv;
out vec4 FragColor;

layout(std430, binding = 6) readonly buffer PBRDeferredClusterBuffer
{
	ivec4 deferredClusterOffsetCount[];
};

uniform int clusteredLightTileSize;
uniform int clusteredLightGridColumns;
uniform int clusteredLightGridRows;
uniform int clusteredLightDepthSlices;
uniform int clusteredLightDebugDepthSlice;
uniform int clusteredLightDebugMaxLights;
uniform float clusteredLightDebugIntensity;

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

int clusterIndexFor(int tileX, int tileY, int depthSlice)
{
	return (depthSlice * clusteredLightGridRows + tileY) * clusteredLightGridColumns + tileX;
}

void main()
{
	if (clusteredLightGridColumns <= 0 || clusteredLightGridRows <= 0 || clusteredLightDepthSlices <= 0 || clusteredLightTileSize <= 0)
	{
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	ivec2 tileCoord = ivec2(gl_FragCoord.xy) / clusteredLightTileSize;
	tileCoord = clamp(tileCoord, ivec2(0), ivec2(clusteredLightGridColumns - 1, clusteredLightGridRows - 1));

	int lightCount = 0;
	if (clusteredLightDebugDepthSlice >= 0)
	{
		int depthSlice = clamp(clusteredLightDebugDepthSlice, 0, clusteredLightDepthSlices - 1);
		int clusterIndex = clusterIndexFor(tileCoord.x, tileCoord.y, depthSlice);
		lightCount = max(deferredClusterOffsetCount[clusterIndex].y, 0);
	}
	else
	{
		for (int depthSlice = 0; depthSlice < clusteredLightDepthSlices; ++depthSlice)
		{
			int clusterIndex = clusterIndexFor(tileCoord.x, tileCoord.y, depthSlice);
			lightCount = max(lightCount, max(deferredClusterOffsetCount[clusterIndex].y, 0));
		}
	}

	float normalizedCount = float(lightCount) / float(max(clusteredLightDebugMaxLights, 1));
	vec3 color = heatmap(normalizedCount) * max(clusteredLightDebugIntensity, 0.0);
	FragColor = vec4(color, 1.0);
}
