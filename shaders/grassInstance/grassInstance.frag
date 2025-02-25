#version 460 core

uniform float time;
uniform sampler2D samplerGrass;
uniform sampler2D MaskSampler;
uniform sampler2D opacityMask;
uniform sampler2D cloudMask;
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


//草地贴图特性
uniform float uvScale;
uniform float brightness;

//云层贴图特性
uniform vec3 cloudWhiteColor;
uniform vec3 cloudBlackColor;
uniform float cloudUVScale;
uniform float cloudSpeed;
uniform vec3 windDirection;
uniform float cloudLerp;
//text b
uniform float part;

uniform float shiness;

in vec4 color;
in vec2 uv;//2
in vec3 normal;
in vec3 worldPosition;
in vec2 worldXZ;
out vec4 FragColor;
out float depth;

struct DirectionalLight
{
	vec3 direction;
	vec3 color;
	float specularIntensity;
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

vec3 calculateSpotLight(vec3 objectColor,SpotLight light,vec3 normal,vec3 viewDir)
{
	vec3 lightDir = normalize(worldPosition - light.position);
	vec3 targetDirN = normalize(light.targetDirection);
	
	//计算spotlight的照射范围
    float cGamma = dot(lightDir,targetDirN);
    float intensity = clamp((cGamma-light.outLine)/(light.innerLine - light.outLine),0.0f,1.0f);

	
	vec3 diffuseColor = calculateDiffuse(light.color,objectColor,lightDir,normal);
	vec3 specularColor = calculateSpecular(light.color,lightDir,normal,viewDir,light.specularIntensity);

	return (diffuseColor + specularColor ) * intensity;
}

vec3 calculateDirectionalLight(vec3 objectColor, DirectionalLight light, vec3 normal,vec3 viewDir)
{
	vec3 lightDir = normalize(light.direction);
	
	vec3 diffuseColor = calculateDiffuse(light.color,objectColor,lightDir,normal);
	vec3 specularColor = calculateSpecular(light.color,lightDir,normal,viewDir,light.specularIntensity);

	return diffuseColor + specularColor;
}

vec3 calculatePointLight(vec3 objectColor,PointLight light, vec3 normal,vec3 viewDir)
{
	
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
	vec2 worldUV = worldXZ / uvScale;
	vec2 cloudUV = worldXZ / cloudUVScale;
	vec3 res = vec3(0.0f,0.0f,0.0f);
	vec3 objectColor = texture(samplerGrass,worldUV).xyz * brightness;
	float alpha = texture(opacityMask,uv).r;

	vec3 lightDirN = normalize(worldPosition - spotLight.position);
	vec3 targetDirN = normalize(spotLight.targetDirection);
	vec3 viewDir = normalize(worldPosition - cameraPosition);
	if(alpha==0)
	{
		discard;
	}
	//环境光计算
	vec3 ambientColor = objectColor * ambientColor;
	res += calculateSpotLight(objectColor,spotLight,normal,viewDir);
	res += calculateDirectionalLight(objectColor,directionalLight,normal,viewDir);
	for(int i = 0;i<POINT_LIGHT_NUM;i++) {
		res += calculatePointLight(objectColor,pointLights[i],normal,viewDir);
	}
	vec3 grassColor = res + ambientColor;

	/*
	float Zndc = gl_FragCoord.z * 2.0f - 1.0f;
	float depth = 2.0f * near / (far + near - Zndc * (far - near));
	finalColor = vec3(depth,depth,depth);
	*/
	cloudUV = cloudUV + time * cloudSpeed * windDirection.xz;
	float couldMask = texture(cloudMask,cloudUV).r;
	vec3 cloudColor = mix(cloudBlackColor,cloudWhiteColor,couldMask);
	vec3 finalColor = mix(grassColor,cloudColor,cloudLerp);
	
	FragColor = vec4(finalColor,alpha * opacity);


	


}