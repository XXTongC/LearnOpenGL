#version 460 core
uniform float time;
uniform sampler2D samplerGrass;
uniform sampler2D samplerLand;
uniform sampler2D samplerNoise;
//光源参数
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 ambientColor;
//相机世界位置
uniform vec3 cameraPosition;
//光照强度控制参数
uniform float specularIntensity;

in vec4 color;
in vec2 uv;//2
in vec3 normal;
in vec3 worldPosition;
out vec4 FragColor;

void main()
{
	vec3 grassColor = texture(samplerGrass,uv).xyz;
	vec3 landColor = texture(samplerLand,uv).xyz;
	vec3 noiseColor = texture(samplerNoise,uv).xyz;
	float weight = noiseColor.r * abs(sin(time/3));
	vec3 objectColor = mix(grassColor,landColor,weight);
	/* 手动mix
	vec4 finalColor = grassColor * (1 - weight) + landColor * weight;
	*/
	//自动mix, weight对应第二个参数
	//环境光计算
	vec3 ambientColor = objectColor * ambientColor;
	//求反射角
	vec3 lightDirN = normalize(lightDirection);
	
	//求viewDirction
	vec3 viewDir = normalize(worldPosition - cameraPosition);
	
	//计算diffuse
	float diffuse = clamp(dot(-lightDirN,normal),0.0,1.0);
	vec3 diffuseColor = lightColor * diffuse * objectColor;
	//计算specular
	//防止背面穿透光的效果
	float dotResult = dot(-lightDirN,normal) ; 
	float flag = step(0.0, dotResult);

	vec3 lightReflect = normalize(reflect(lightDirN,normal));
	float specular = max(dot(lightReflect,-viewDir),0.0); 



	//控制光斑大小
	specular = pow(specular,16);
	//这里没有乘以材质的颜色的原因是反射光一般是以入射光本身为主，更符合现实中的物理现象
	vec3 specularColor = lightColor * specular * flag * specularIntensity;

	//finalColor = finalColor * clamp(vec4(normal ,1.0f ),0.0,1.0);
	
	vec3 finalColor = diffuseColor + specularColor + ambientColor;
	
	FragColor = vec4(finalColor,1.0);
}