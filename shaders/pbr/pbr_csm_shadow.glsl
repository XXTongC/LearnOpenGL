const int MAX_CSM_LAYERS = 20;

uniform sampler2DArray shadowMapSampler;
uniform int csmLayerCount;
uniform float csmLayers[MAX_CSM_LAYERS];
uniform mat4 lightMatrices[MAX_CSM_LAYERS];
uniform float bias;
uniform float pcfRadius;
uniform mat4 viewMatrix;

float getShadowBias(vec3 n, vec3 lightDir)
{
	return max(bias * (1.0 - dot(normalize(lightDir), normalize(n))), 0.0005);
}

int getCsmLayer(vec3 positionWorldSpace)
{
	float z = -(viewMatrix * vec4(positionWorldSpace, 1.0)).z;
	int layer = max(csmLayerCount - 1, 0);
	for (int i = 0; i < MAX_CSM_LAYERS - 1; ++i)
	{
		if (i >= csmLayerCount)
		{
			break;
		}
		if (z <= csmLayers[i + 1])
		{
			layer = i;
			break;
		}
	}
	return clamp(layer, 0, max(csmLayerCount - 1, 0));
}

float calculateCsmShadow(vec3 positionWorldSpace, vec3 n, vec3 lightDir)
{
	if (csmLayerCount <= 0)
	{
		return 0.0;
	}

	int layer = getCsmLayer(positionWorldSpace);
	vec4 lightSpaceClipCoord = lightMatrices[layer] * vec4(positionWorldSpace, 1.0);
	vec3 projectCoord = lightSpaceClipCoord.xyz / lightSpaceClipCoord.w;
	projectCoord = projectCoord * 0.5 + 0.5;

	if (projectCoord.z < 0.0 || projectCoord.z > 1.0 ||
		projectCoord.x < 0.0 || projectCoord.x > 1.0 ||
		projectCoord.y < 0.0 || projectCoord.y > 1.0)
	{
		return 0.0;
	}

	vec2 texelSize = 1.0 / vec2(textureSize(shadowMapSampler, 0).xy);
	float radius = max(pcfRadius, 1.0);
	float shadow = 0.0;
	float currentDepth = projectCoord.z - getShadowBias(n, lightDir);

	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			vec2 offset = vec2(float(x), float(y)) * texelSize * radius;
			float closestDepth = texture(shadowMapSampler, vec3(projectCoord.xy + offset, float(layer))).r;
			shadow += currentDepth > closestDepth ? 1.0 : 0.0;
		}
	}

	return shadow / 9.0;
}
