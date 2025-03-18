#version 460 core

uniform float time;
uniform sampler2D samplerGrass;
uniform sampler2D MaskSampler;
uniform sampler2D NormalMapSampler;
uniform sampler2D ParallaxMapSampler;
//光源参数
uniform vec3 ambientColor;
//相机世界位置
uniform vec3 cameraPosition;

uniform float heightScale;
uniform int layerNum;

//透明度设置
uniform float opacity;

//光照强度控制参数

//depth test
//uniform float near;
//uniform float far;

//text b
uniform float part;

uniform float shiness;

in vec4 color;
in vec2 uv;//2
in vec3 normal;
in vec3 worldPosition;
in mat3 TBN;
out vec4 FragColor;

#include "../common/commonLight.glsl"

uniform SpotLight spotLight;
uniform DirectionalLight directionalLight;
#define POINT_LIGHT_NUM 4
uniform PointLight pointLights[POINT_LIGHT_NUM];
//计算漫反射光照

//	uv: current pixcel uv
//	viewDir: viewDirection between current pixcel and camera world space 
vec2 parallaxUV(vec2 uv,vec3 viewDir)
{
	viewDir = normalize(transpose(TBN) * viewDir);
	float layerDepth = 1.0f/layerNum;
	vec2 deltaTexCoords;
	float lastLenth = 0.0f;
	deltaTexCoords = viewDir.xy / viewDir.z * heightScale / layerNum;
	
	vec2 currentTexCoords = uv;
	float currentSampleValue = texture(ParallaxMapSampler,currentTexCoords).r;
	float currentLayerDepth = 0.0f;
	while(currentLayerDepth<currentSampleValue)
	{
		lastLenth = currentSampleValue - currentLayerDepth;
		currentTexCoords -= deltaTexCoords;
		currentSampleValue = texture(ParallaxMapSampler,currentTexCoords).r;
		currentLayerDepth += layerDepth;
	}
	if(lastLenth>0.0f)
	{
		float currentLenth = currentLayerDepth - currentSampleValue;
		currentTexCoords = currentTexCoords + currentLenth/(lastLenth + currentLenth) * deltaTexCoords;
	}

	return currentTexCoords;

}

void main()
{
	vec3 viewDir = normalize(worldPosition - cameraPosition);
	vec2 uvReal = parallaxUV(uv,viewDir);
	vec3 res = vec3(0.0f,0.0f,0.0f);
	vec3 objectColor = texture(samplerGrass,uvReal).xyz;
	float alpha = texture(samplerGrass,uvReal).a;
	
	vec3 lightDirN = normalize(worldPosition - spotLight.position);
	vec3 targetDirN = normalize(spotLight.targetDirection);
	
	vec3 normalN = texture(NormalMapSampler,uvReal).rgb;
	normalN = normalN * 2.0f - vec3(1.0f);
	normalN = normalize(TBN * normalN);
	//环境光计算
	vec3 ambientColor = objectColor * ambientColor;
	res += calculateSpotLight(spotLight,normalN,viewDir);
	res += calculateDirectionalLight(directionalLight,normalN,viewDir);
	for(int i = 0;i<POINT_LIGHT_NUM;i++) {
		res += calculatePointLight(pointLights[i],normalN,viewDir);
	}
	vec3 finalColor = res + ambientColor;

	/*
	float Zndc = gl_FragCoord.z * 2.0f - 1.0f;
	float depth = 2.0f * near / (far + near - Zndc * (far - near));
	finalColor = vec3(depth,depth,depth);
	*/
	FragColor = vec4(finalColor,alpha * opacity);
	//FragColor = vec4(texture(samplerGrass,uv).rgb,alpha * opacity);
}