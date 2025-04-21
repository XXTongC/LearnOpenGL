#version 460 core

#define NUM_SAMPLER 32
#define PI 3.141592653589793
#define PI2 6.283185307179586
uniform float time;
uniform sampler2D samplerGrass;
uniform sampler2D MaskSampler;

uniform sampler2DArray shadowMapSampler;
//光源参数
uniform vec3 ambientColor;
//相机世界位置
uniform vec3 cameraPosition;
uniform float bias;
uniform mat4 viewMatrix;

//透明度设置
uniform float opacity;

//光照强度控制参数
//text b
uniform float part;
uniform float shiness;
// PCSS relative
uniform float lightSize;
uniform float frustum;
uniform float nearPlane;


in vec4 color;
in vec2 uv;//2
in vec3 normal;
in vec3 worldPosition;

out vec4 FragColor;

#include "../common/commonLight.glsl"

uniform SpotLight spotLight;
uniform DirectionalLight directionalLight;
#define POINT_LIGHT_NUM 4
uniform PointLight pointLights[POINT_LIGHT_NUM];

vec2 disk[NUM_SAMPLER];
uniform float diskTightness;
uniform float pcfRadius;

//CSM
uniform int csmLayerCount;
uniform float csmLayers[20];
uniform mat4 lightMatrices[20];

float rand_2to1(vec2 uv)
{
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}

void poissonDiskSampler(vec2 randomSeed){
	//	1. 初始弧度
	float angle = rand_2to1(randomSeed) * PI2;
	//	2. 初始半径
	float radius = 1.0 / float(NUM_SAMPLER);
	//	3. 弧度步长
	float angleStep = 3.883222077450933;
	//	4. 半径步长
	float radiusStep = radius;
	//	5. 循环生成
	for(int i = 0;i<NUM_SAMPLER;++i)
	{
		disk[i] = vec2(cos(angle),sin(angle)) * pow(radius,diskTightness);
		angle += angleStep;
		radius += radiusStep;
	}
}



float getBias(vec3 normal,vec3 lightDir)
{
	vec3 normalN = normalize(normal);
	vec3 lightDirN = normalize(lightDir);

	return max(bias * (1- dot(lightDirN,normalN)),0.0005);
}
/*
float calculateShadow(vec3 normal,vec3 lightDir)
{
    //  1. 找到当前像素在光源空间内的NDC坐标
    vec3 lightNDC = lightSpaceClipCoord.xyz/lightSpaceClipCoord.w;

    //  2. 找到当前像素在ShadowMap上的UV
    vec3 projectCoord = lightNDC * 0.5 + 0.5;
    vec2 uv = projectCoord.xy;

    //  3. 使用这个uv对SM进行采样，得到ClosestDepth
    float ClosestDepth = texture(shadowMapSampler,uv).r;

    //  4. 对比当前像素在光源空间内的深度值与ClosestDepth的大小
    float selfDepth = projectCoord.z;
    float shadow = (selfDepth - getBias(normal,lightDir)) > ClosestDepth?1.0f:0.0f;
    return shadow;
}
*/
//float pcf(vec3 normal,vec3 lightDir, float pcfUVRadius);
float pcfSimplify(vec4 lightSpaceClipCoord , int layer,vec3 normal,vec3 lightDir, float pcfUVRadius);

float findBlocker(vec3 lightSpacePosition, vec2 shadowUV, float depthReceiver, vec3 normal, vec3 lightDir,int layer)
{
	poissonDiskSampler(shadowUV);
	
	float searchRadius = (-lightSpacePosition.z - nearPlane) / (-lightSpacePosition.z) * lightSize;
	float searchRadiusUV = searchRadius / frustum;

	int blockNum = 0;
	float blockSumDepth = 0.0f;
	for(int i = 0;i<NUM_SAMPLER;++i)
	{
		float samplerDepth = texture(shadowMapSampler,vec3(shadowUV + disk[i] * searchRadiusUV,layer)).r;
		if(depthReceiver - getBias(normal,lightDir)>samplerDepth)
		{
			++blockNum;
			blockSumDepth += samplerDepth;
		}
	}
	return blockNum!=0?blockSumDepth / blockNum:-1.0;

}

float pcss(vec3 lightSpacePosition, vec4 lightSpaceClipCoord, vec3 normal,vec3 lightDir);
int getCurrentLayer(vec3 positionWorldSpace);
float csm(vec3 positionWorldSpace, vec3 normal, vec3 lightDir, float pcfRadius);

void main()
{
	vec3 res = vec3(0.0f,0.0f,0.0f);
	vec3 objectColor = texture(samplerGrass,uv).xyz;
	float alpha = texture(samplerGrass,uv).a;


	vec3 lightDirN = normalize(worldPosition - spotLight.position);
	vec3 targetDirN = normalize(spotLight.targetDirection);
	vec3 viewDir = normalize(worldPosition - cameraPosition);
	
	//环境光计算
	vec3 ambientColor = objectColor * ambientColor;
	res += calculateSpotLight(spotLight,normal,viewDir);
	res += calculateDirectionalLight(directionalLight,normal,viewDir);
	for(int i = 0;i<POINT_LIGHT_NUM;i++) {
		res += calculatePointLight(pointLights[i],normal,viewDir);
	}

    float shadow = csm(worldPosition,normal,-directionalLight.direction,pcfRadius);
	vec3 finalColor = res * (1.0 - shadow) + ambientColor;
	
	/*
	float Zndc = gl_FragCoord.z * 2.0f - 1.0f;
	float depth = 2.0f * near / (far + near - Zndc * (far - near));
	finalColor = vec3(depth,depth,depth);
	*/
	//---text---

	//----------
	
	FragColor = vec4(finalColor,alpha * opacity);
	//FragColor = vec4(shadow,shadow,shadow,alpha * opacity);

}
/*
float pcf(vec3 normal,vec3 lightDir, float pcfUVRadius)
{
	vec3 lightNDC = lightSpaceClipCoord.xyz/lightSpaceClipCoord.w;

	vec3 lightProjectCoord = lightNDC * 0.5 +0.5;
	vec2 texelSize = 1.0f/textureSize(shadowMapSampler,0);
	vec2 uv = lightProjectCoord.xy;
	float depth = lightProjectCoord.z;
	poissonDiskSampler(uv);
	float sum = 0.0;
	
	for(int i = 0;i<NUM_SAMPLER;++i)
	{
		float closestDepth = texture(shadowMapSampler,uv + disk[i] * pcfUVRadius).r;
		sum += (closestDepth<(depth - getBias(normal,lightDir))?1.0f:0.0f);
	}
	
	return sum/NUM_SAMPLER;
}
*/
float pcfSimplify(vec4 lightSpaceClipCoord ,int layer,vec3 normal,vec3 lightDir, float pcfUVRadius)
{
	vec3 lightNDC = lightSpaceClipCoord.xyz/lightSpaceClipCoord.w;

	vec3 lightProjectCoord = lightNDC * 0.5 +0.5;
	//vec2 texelSize = 1.0f/textureSize(shadowMapSampler,0);
	vec2 uv = lightProjectCoord.xy;
	float depth = lightProjectCoord.z;
	poissonDiskSampler(uv);
	float sum = 0.0;
	
	for(int i = 0;i<NUM_SAMPLER;++i)
	{
		float closestDepth = texture(shadowMapSampler,vec3(uv + disk[i] * pcfUVRadius,layer)).r;
		sum += (closestDepth<(depth - getBias(normal,lightDir))?1.0f:0.0f);
	}
	
	return sum/NUM_SAMPLER;
}

float pcss(vec4 lightSpaceClipCoord, vec3 lightSpacePosition, int layer, vec3 normal, vec3 lightDir, float pcfRadius)
{
	// 	1. 获取当前像素在ShadowMap下的uv以及在光源坐标系中的深度值
	vec3 lightNDC = lightSpaceClipCoord.xyz/lightSpaceClipCoord.w;
	vec3 lightProjectCoord = lightNDC * 0.5 +0.5;
	vec2 uv = lightProjectCoord.xy;
	float depth = lightProjectCoord.z;

	// 	2. 计算dBlock
	float dBlocker = findBlocker(lightSpacePosition,uv,depth,normal,-lightDir,layer);

	// 	3. 计算penumbra
	float penumbra =  (dBlocker > 0.0) ? ((depth - dBlocker) / dBlocker) * (lightSize / frustum) : 0.0;

	//	4. 计算pcfRadius
	return pcfSimplify(lightSpaceClipCoord, layer, normal, lightDir, penumbra * pcfRadius);
}

int getCurrentLayer(vec3 positionWorldSpace)
{
	vec3 positionCameraSpace = (viewMatrix * vec4(positionWorldSpace,1.0)).xyz;
	float z = -positionCameraSpace.z;
	int layer = 0;

	for(int i = 0;i<=csmLayerCount;++i)
	{
		if(z<csmLayers[i])
		{
			layer = i-1;
			break;
		}
	}
	return layer;
}

float csm(vec3 positionWorldSpace, vec3 normal, vec3 lightDir, float pcfRadius)
{
	int layer = getCurrentLayer(positionWorldSpace);
	vec4 lightSpaceClipCoord = lightMatrices[layer] * vec4(positionWorldSpace,1.0);
	vec3 lightSpacePosition = lightSpaceClipCoord.xyz / lightSpaceClipCoord.w;
	return pcfSimplify(lightSpaceClipCoord, layer, normal,lightDir,pcfRadius);
}