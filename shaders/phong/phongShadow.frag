#version 460 core

uniform float time;
uniform sampler2D samplerGrass;
uniform sampler2D MaskSampler;
uniform sampler2D shadowMapSampler;
//光源参数
uniform vec3 ambientColor;
//相机世界位置
uniform vec3 cameraPosition;

//透明度设置
uniform float opacity;

//光照强度控制参数
//text b
uniform float part;
uniform float shiness;

in vec4 color;
in vec2 uv;//2
in vec3 normal;
in vec3 worldPosition;
in vec4 lightSpaceClipCoord;

out vec4 FragColor;

#include "../common/commonLight.glsl"

uniform SpotLight spotLight;
uniform DirectionalLight directionalLight;
#define POINT_LIGHT_NUM 4
uniform PointLight pointLights[POINT_LIGHT_NUM];

float calculateShadow()
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
    float shadow = selfDepth>ClosestDepth?1.0f:0.0f;
    return shadow;
}

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

    float shadow = calculateShadow();
	vec3 finalColor = res * (1.0 - shadow) + ambientColor;

	/*
	float Zndc = gl_FragCoord.z * 2.0f - 1.0f;
	float depth = 2.0f * near / (far + near - Zndc * (far - near));
	finalColor = vec3(depth,depth,depth);
	*/
	FragColor = vec4(finalColor,alpha * opacity);
	//FragColor = vec4(texture(shadowMapSampler,uv).xyz,alpha * opacity);

}