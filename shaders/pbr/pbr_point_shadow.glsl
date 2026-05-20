const int MAX_PBR_POINT_SHADOWS = 16;

uniform sampler2DArray pbrPointShadowMapSampler;
uniform int pbrPointShadowAtlasEnabled;
uniform int pbrPointShadowAtlasLightCount;
uniform float pbrPointShadowLayerBase[MAX_PBR_POINT_SHADOWS];
uniform float pbrPointShadowNear[MAX_PBR_POINT_SHADOWS];
uniform float pbrPointShadowFar[MAX_PBR_POINT_SHADOWS];
uniform float pbrPointShadowBias[MAX_PBR_POINT_SHADOWS];
uniform float pbrPointShadowPcfRadius[MAX_PBR_POINT_SHADOWS];
uniform float pbrPointShadowEnabled[MAX_PBR_POINT_SHADOWS];

bool getPbrPointShadowUv(vec3 sampleDir, out vec2 shadowUv, out int faceIndex)
{
	vec3 absDir = abs(sampleDir);
	float maxComp = max(max(absDir.x, absDir.y), absDir.z);
	float depthScale = maxComp;

	if (depthScale <= 0.0001)
	{
		return false;
	}

	if (maxComp == absDir.x)
	{
		shadowUv = sampleDir.x > 0.0 ? vec2(-sampleDir.z, -sampleDir.y) : vec2(sampleDir.z, -sampleDir.y);
		faceIndex = sampleDir.x > 0.0 ? 0 : 1;
	}
	else if (maxComp == absDir.y)
	{
		shadowUv = sampleDir.y > 0.0 ? vec2(sampleDir.x, sampleDir.z) : vec2(sampleDir.x, -sampleDir.z);
		faceIndex = sampleDir.y > 0.0 ? 2 : 3;
	}
	else
	{
		shadowUv = sampleDir.z > 0.0 ? vec2(sampleDir.x, -sampleDir.y) : vec2(-sampleDir.x, -sampleDir.y);
		faceIndex = sampleDir.z > 0.0 ? 4 : 5;
	}

	shadowUv = shadowUv / depthScale * 0.5 + 0.5;
	return shadowUv.x >= 0.0 && shadowUv.x <= 1.0 && shadowUv.y >= 0.0 && shadowUv.y <= 1.0;
}

float calculatePbrPointShadow(vec3 positionWorldSpace, vec3 lightPosition, int lightIndex)
{
	if (pbrPointShadowAtlasEnabled != 1 ||
		lightIndex < 0 ||
		lightIndex >= pbrPointShadowAtlasLightCount ||
		lightIndex >= MAX_PBR_POINT_SHADOWS ||
		pbrPointShadowEnabled[lightIndex] < 0.5)
	{
		return 0.0;
	}

	vec3 fragToLight = positionWorldSpace - lightPosition;
	float currentDepth = length(fragToLight);
	float farPlane = max(pbrPointShadowFar[lightIndex], 0.0001);
	if (currentDepth <= pbrPointShadowNear[lightIndex] || currentDepth >= farPlane * 1.414)
	{
		return 0.0;
	}

	vec2 shadowUv;
	int faceIndex = 0;
	if (!getPbrPointShadowUv(normalize(fragToLight), shadowUv, faceIndex))
	{
		return 0.0;
	}

	int layerIndex = int(pbrPointShadowLayerBase[lightIndex]) + faceIndex;
	vec2 texelSize = 1.0 / vec2(textureSize(pbrPointShadowMapSampler, 0).xy);
	float radius = max(pbrPointShadowPcfRadius[lightIndex], 1.0);
	float biasDistance = max(pbrPointShadowBias[lightIndex] * farPlane, 0.05);
	float shadow = 0.0;

	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize * radius;
			float closestDepth = texture(pbrPointShadowMapSampler, vec3(shadowUv + offset, float(layerIndex))).r;
			closestDepth *= farPlane * 1.414;
			shadow += currentDepth - biasDistance > closestDepth ? 1.0 : 0.0;
		}
	}

	return shadow / 9.0;
}
