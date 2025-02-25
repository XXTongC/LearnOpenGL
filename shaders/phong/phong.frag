#version 460 core

uniform float time;
uniform sampler2D samplerGrass;
uniform sampler2D MaskSampler;
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

struct DirectionalLight
{
	vec3 direction;
	vec3 color;
	float specularIntensity;
	float intensity;
};

struct PointLight
{
	vec3 position;
	vec3 color;
	float specularIntensity;

	float k2;
	float k1;
	float k0;
};

struct SpotLight
{
	vec3 position;
	vec3 targetDirection;
	vec3 color;
	float outLine;
	float innerLine;
	float specularIntensity;
};

uniform SpotLight spotLight;
uniform DirectionalLight directionalLight;
#define POINT_LIGHT_NUM 4
uniform PointLight pointLights[POINT_LIGHT_NUM];
//计算漫反射光照
vec3 calculateDiffuse(vec3 lightColor,vec3 objectColor,vec3 lightDir,vec3 normal)
{
	//计算diffuse
	float diffuse = clamp(dot(-lightDir,normal),0.0,1.0);
	vec3 diffuseColor = lightColor * diffuse * objectColor;

	return diffuseColor;
}
//计算镜面反射光照
vec3 calculateSpecular(vec3 lightColor,vec3 lightDir,vec3 normal,vec3 viewDir,float intensity)
{

	//防止背面穿透光的效果
	float dotResult = dot(-lightDir,normal) ; 
	float flag = step(0.0, dotResult);
	vec3 lightReflect = normalize(reflect(lightDir,normal));
	//计算specular
	float specular = max(dot(lightReflect,-viewDir),0.0); 

	//控制光斑大小
	specular = pow(specular,shiness);
	//蒙版值计算
	float specularMask = texture(MaskSampler,uv).r;

	//这里没有乘以材质的颜色的原因是反射光一般是以入射光本身为主，更符合现实中的物理现象
	vec3 specularColor = lightColor * specular * flag * intensity * specularMask;
	//vec3 specularColor = lightColor * specular * flag * intensity;

	return specularColor;
}

vec3 calculateSpotLight(SpotLight light,vec3 normal,vec3 viewDir)
{
	vec3 objectColor = texture(samplerGrass,uv).xyz;
	vec3 lightDir = normalize(worldPosition - light.position);
	vec3 targetDirN = normalize(light.targetDirection);
	
	//计算spotlight的照射范围
    float cGamma = dot(lightDir,targetDirN);
    float intensity = clamp((cGamma-light.outLine)/(light.innerLine - light.outLine),0.0f,1.0f);

	
	vec3 diffuseColor = calculateDiffuse(light.color,objectColor,lightDir,normal);
	vec3 specularColor = calculateSpecular(light.color,lightDir,normal,viewDir,light.specularIntensity);

	return (diffuseColor + specularColor ) * intensity;
}

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal,vec3 viewDir)
{
	light.color *= light.intensity;
	vec3 objectColor = texture(samplerGrass,uv).xyz;
	vec3 lightDir = normalize(light.direction);
	
	vec3 diffuseColor = calculateDiffuse(light.color,objectColor,lightDir,normal);
	vec3 specularColor = calculateSpecular(light.color,lightDir,normal,viewDir,light.specularIntensity);

	return diffuseColor + specularColor;
}

vec3 calculatePointLight(PointLight light, vec3 normal,vec3 viewDir)
{
	vec3 objectColor = texture(samplerGrass,uv).xyz;
	vec3 lightDir = normalize(worldPosition - light.position);
	
	//计算衰减
	float dist = length(worldPosition - light.position);
    float attenuation = 1.0f / (light.k2 * dist * dist + light.k1 * dist + light.k0);

	vec3 diffuseColor = calculateDiffuse(light.color,objectColor,lightDir,normal);
	vec3 specularColor = calculateSpecular(light.color,lightDir,normal,viewDir,light.specularIntensity);

	return (diffuseColor + specularColor ) * attenuation;
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
	vec3 finalColor = res + ambientColor;

	/*
	float Zndc = gl_FragCoord.z * 2.0f - 1.0f;
	float depth = 2.0f * near / (far + near - Zndc * (far - near));
	finalColor = vec3(depth,depth,depth);
	*/
	FragColor = vec4(finalColor,alpha * opacity);
}