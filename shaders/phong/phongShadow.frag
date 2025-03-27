#version 460 core

#define NUM_SAMPLER 32
#define PI 3.141592653589793
#define PI2 6.283185307179586
uniform float time;
uniform sampler2D samplerGrass;
uniform sampler2D MaskSampler;
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

float pcf(vec3 normal,vec3 lightDir);

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

    float shadow = pcf(normal,-directionalLight.direction);
	vec3 finalColor = res * (1.0 - shadow) + ambientColor;

	/*
	float Zndc = gl_FragCoord.z * 2.0f - 1.0f;
	float depth = 2.0f * near / (far + near - Zndc * (far - near));
	finalColor = vec3(depth,depth,depth);
	*/
	FragColor = vec4(finalColor,alpha * opacity);
	//FragColor = vec4(texture(shadowMapSampler,uv).xyz,alpha * opacity);

}

float pcf(vec3 normal,vec3 lightDir)
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
		float closestDepth = texture(shadowMapSampler,uv + disk[i] * pcfRadius).r;
		sum += (closestDepth<(depth - getBias(normal,lightDir))?1.0f:0.0f);
	}
	
	/*
	for(int x = -1;x<=1;++x)
	{
		for(int y = -1;y<=1;++y)
		{
			float closestDepth = texture(shadowMapSampler,uv + vec2(x,y) * texelSize).r;
			sum += (closestDepth<(depth - getBias(normal,lightDir))?1.0f:0.0f);
		}
	}
	*/
	//return sum/9.0;
	return sum/NUM_SAMPLER;
}


