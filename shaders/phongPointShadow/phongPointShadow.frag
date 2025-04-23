#version 460 core

#define NUM_SAMPLER 8
#define PI 3.141592653589793
#define PI2 6.283185307179586
uniform float time;
uniform sampler2D samplerGrass;
uniform sampler2D MaskSampler;
uniform sampler2DArray pointShadowMaps;
uniform sampler2D shadowMapSampler;
//光源参数
uniform vec3 ambientColor;
//相机世界位置
uniform vec3 cameraPosition;
uniform float bias;

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
in vec4 directionalLightSpacePos; // 新的输入，替换 lightSpaceClipCoord
in vec3 lightSpacePosition;

out vec4 FragColor;

#include "../common/commonLight.glsl"

uniform SpotLight spotLight;
uniform DirectionalLight directionalLight;
uniform int POINT_LIGHT_NUM;
uniform PointLight pointLights[20];

float findBlockerDistance_PointLight(vec2 uv, float currentDepth, int layerIndex, float searchWidth, PointLight light);
float PCSS_PointLight(float currentDepth, vec2 uv, int layerIndex, PointLight light);


vec2 disk[NUM_SAMPLER];
uniform float diskTightness;
uniform float pcfRadius;
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



float pcf(vec3 normal,vec3 lightDir, float pcfUVRadius);
float findBlocker(vec3 lightSpacePosition, vec2 shadowUV, float depthReceiver, vec3 normal, vec3 lightDir)
{
	poissonDiskSampler(shadowUV);
	
	float searchRadius = (-lightSpacePosition.z - nearPlane) / (-lightSpacePosition.z) * lightSize;
	float searchRadiusUV = searchRadius / frustum;

	int blockNum = 0;
	float blockSumDepth = 0.0f;
	for(int i = 0;i<NUM_SAMPLER;++i)
	{
		float samplerDepth = texture(shadowMapSampler,shadowUV + disk[i] * searchRadiusUV).r;
		if(depthReceiver - getBias(normal,lightDir)>samplerDepth)
		{
			++blockNum;
			blockSumDepth += samplerDepth;
		}
	}
	return blockNum!=0?blockSumDepth / blockNum:-1.0;

}


float ShadowCalculationPointLight(vec3 fragPos, PointLight light, int shadowMapIndex)
{
    // 获取片段到光源的向量
    vec3 fragToLight = fragPos - light.position;
    
    // 计算当前片段在光源空间的深度
    float currentDepth = length(fragToLight);
    
    // 获取立方体贴图的采样向量
    vec3 sampleDir = normalize(fragToLight);
    
    // 确定立方体贴图的哪个面
    vec3 absDir = abs(sampleDir);
    float maxComp = max(max(absDir.x, absDir.y), absDir.z);
    
    // 计算2D纹理坐标和面索引
    vec2 uv;
    float depthScale;
    int faceIndex;
    
    if (maxComp == absDir.x) {
        uv = sampleDir.x > 0 ? vec2(-sampleDir.z, -sampleDir.y) : vec2(sampleDir.z, -sampleDir.y);
        faceIndex = sampleDir.x > 0 ? 0 : 1;
        depthScale = absDir.x;
    } else if (maxComp == absDir.y) {
        uv = sampleDir.y > 0 ? vec2(sampleDir.x, sampleDir.z) : vec2(sampleDir.x, -sampleDir.z);
        faceIndex = sampleDir.y > 0 ? 2 : 3;  // 3 号贴图朝下
        depthScale = absDir.y;
    } else {
        uv = sampleDir.z > 0 ? vec2(sampleDir.x, -sampleDir.y) : vec2(-sampleDir.x, -sampleDir.y);
        faceIndex = sampleDir.z > 0 ? 4 : 5;
        depthScale = absDir.z;
    }
    
    uv = uv / depthScale * 0.5 + 0.5;
    
    // 计算在Texture2DArray中的层索引
    int layerIndex = shadowMapIndex * 6 + faceIndex;
    return PCSS_PointLight(currentDepth, uv, layerIndex, light);

    /*
    // 从深度贴图数组中采样
    float closestDepth = texture(pointShadowMaps, vec3(uv, layerIndex)).r;
    
    // 将采样得到的深度值转换为线性深度
    closestDepth *= light.far * 1.414;
    
    // 应用偏移以避免阴影失真
    float bias = 1;  // 使用一个固定的小值

    //float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    poissonDiskSampler(uv);
	float sum = 0.0;
	
	for(int i = 0;i<NUM_SAMPLER;++i)
	{
        float closestDepth = texture(pointShadowMaps, vec3(uv + disk[i] * pcfRadius, layerIndex)).r;
		closestDepth *= light.far * 1.414;
        sum += (closestDepth<(currentDepth - max(bias * (1- dot(normalize(fragToLight),normal)),1))?1.0f:0.0f);
	}
	
	return sum/NUM_SAMPLER;
    //return shadow;
    */
}

vec3 calculatePointLightWithShadow(PointLight light, vec3 normal, vec3 viewDir, int shadowMapIndex)
{
    vec3 result = calculatePointLight(light, normal, viewDir);
    float shadow = ShadowCalculationPointLight(worldPosition, light,shadowMapIndex);
    return result * (1.0 - shadow);
}
float calculateDirectionalShadow(vec3 normal, vec3 lightDir)
{
    vec3 projCoords = directionalLightSpacePos.xyz / directionalLightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    float currentDepth = projCoords.z;
    float closestDepth = texture(shadowMapSampler, projCoords.xy).r;
    
    float bias = getBias(normal, lightDir);
    return (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
}
float pcss(vec3 lightSpacePosition, vec4 lightSpaceClipCoord, vec3 normal,vec3 lightDir);

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
	vec3 directionalResult= calculateDirectionalLight(directionalLight,normal,viewDir);
	float directionalShadow = calculateDirectionalShadow(normal, -directionalLight.direction);
    res += directionalResult * (1.0 - directionalShadow);
    vec3 pointLightContribution = vec3(0.0);
    float pointShadow;
    // 计算点光源
    for(int i = 0; i < POINT_LIGHT_NUM; i++) {
        vec3 pointLightResult = calculatePointLight(pointLights[i], normal, viewDir);
        pointShadow = ShadowCalculationPointLight(worldPosition, pointLights[i], i);
        res += pointLightResult * (1.0 - pointShadow);
        //res += pointLightResult;
        //res +=  pointShadow;

    }

    //res += pointLightContribution;
   // float shadow = pcss(lightSpacePosition,lightSpaceClipCoord,normal,-directionalLight.direction);
	//vec3 finalColor = res * (1.0 - shadow) + ambientColor;
    vec3 finalColor = res  + ambientColor;
	/*
	float Zndc = gl_FragCoord.z * 2.0f - 1.0f;
	float depth = 2.0f * near / (far + near - Zndc * (far - near));
	finalColor = vec3(depth,depth,depth);
	*/
	//---text---

	//----------
    
    FragColor = vec4(finalColor, 1.0);
	//FragColor = vec4(texture(shadowMapSampler,uv).xyz,alpha * opacity);
}

float findBlockerDistance_PointLight(vec2 uv, float currentDepth, int layerIndex, float searchWidth, PointLight light)
{
    float sumBlockerDepth = 0.0;
    int numBlockers = 0;

    for(int i = 0; i < NUM_SAMPLER; i++)
    {
        float shadowMapDepth = texture(pointShadowMaps, vec3(uv + disk[i] * searchWidth, layerIndex)).r;
        shadowMapDepth *= light.far;
        if(shadowMapDepth < currentDepth)
        {
            sumBlockerDepth += shadowMapDepth;
            numBlockers++;
        }
    }

    if(numBlockers == 0) return -1.0;
    return sumBlockerDepth / float(numBlockers);
}

float PCSS_PointLight(float currentDepth, vec2 uv, int layerIndex, PointLight light)
{
    // 1. 寻找平均遮挡深度
    float searchWidth = (currentDepth - light.near) / currentDepth;
    float avgBlockerDepth = findBlockerDistance_PointLight(uv, currentDepth, layerIndex, searchWidth, light);
    if(avgBlockerDepth == -1.0) return 0.0;  // 没有遮挡

    // 2. 计算半影
    float penumbraRatio = (currentDepth - avgBlockerDepth) / avgBlockerDepth;
    float filterRadius = penumbraRatio * light.near / currentDepth;

    // 3. PCF 过滤
    poissonDiskSampler(uv);
	float sum = 0.0;
	
	for(int i = 0;i<NUM_SAMPLER;++i)
	{
        float closestDepth = texture(pointShadowMaps, vec3(uv + disk[i] * filterRadius * pcfRadius, layerIndex)).r;
		closestDepth *= light.far * 1.414;
        sum += (currentDepth - 1 > closestDepth) ? 1.0 : 0.0;
	}
    /*
    float sum = 0.0;
    for(int i = 0; i < NUM_SAMPLER; i++)
    {
        float closestDepth = texture(pointShadowMaps, vec3(uv + disk[i] * filterRadius, layerIndex)).r;
        closestDepth *= light.far * 1.414;
        sum += (currentDepth - 1 > closestDepth) ? 1.0 : 0.0;
    }
    */
    return sum / float(NUM_SAMPLER);
}