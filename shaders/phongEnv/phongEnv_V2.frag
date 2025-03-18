#version 460 core

uniform float time;
uniform sampler2D samplerGrass;
uniform sampler2D MaskSampler;
uniform samplerCube envSampler;
//光源参数
uniform vec3 ambientColor;
//相机世界位置
uniform vec3 cameraPosition;

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
out vec4 FragColor;

#include "../common/commonLight.glsl"

uniform SpotLight spotLight;
uniform DirectionalLight directionalLight;
#define POINT_LIGHT_NUM 4
uniform PointLight pointLights[POINT_LIGHT_NUM];

vec3 calculateEnv(vec3 normal, vec3 viewDir)
{
    vec3 reflectDir = normalize(reflect(viewDir, normal));
    vec3 color = texture(envSampler,reflectDir).rgb;
    return color;
}

void main()
{
	vec3 res = vec3(0.0f,0.0f,0.0f);
	vec3 objectColor = texture(samplerGrass,uv).rgb;
	float alpha = texture(samplerGrass,uv).a;


	vec3 lightDirN = normalize(worldPosition - spotLight.position);
	vec3 targetDirN = normalize(spotLight.targetDirection);
	vec3 viewDir = normalize(worldPosition - cameraPosition);
	
	//环境光计算
	vec3 ambientColor = objectColor * calculateEnv(normalize(normal),viewDir);
	//vec3 ambientColor = objectColor * ambientColor;
    res += calculateSpotLight(spotLight,normal,viewDir);
	res += calculateDirectionalLight(directionalLight,normal,viewDir);
	for(int i = 0;i<POINT_LIGHT_NUM;i++) {
		res += calculatePointLight(pointLights[i],normal,viewDir);
	}
	vec3 finalColor = res + ambientColor;

	/*
	float Zndc = gl_FragCoord.z * 2.0f - 1.0f;
	float depth = 2.0f * near / (far + near - Zndc * (far - near));
	finalColor = vec3(depth,depth,depth);
	*/
	FragColor = vec4(finalColor,alpha * opacity);
}