#version 460 core
uniform float time;
uniform sampler2D samplerGrass;
uniform sampler2D samplerLand;
uniform sampler2D samplerNoise;
in vec4 color;
in vec2 uv;//2
in vec3 normal;
out vec4 FragColor;
void main()
{
	vec4 grassColor = texture(samplerGrass,uv);
	vec4 landColor = texture(samplerLand,uv);
	vec4 noiseColor = texture(samplerNoise,uv);
	
	/* 手动mix
	vec4 finalColor = grassColor * (1 - weight) + landColor * weight;
	*/
	//自动mix, weight对应第二个参数
	float weight = noiseColor.r * abs(sin(time/3));
	vec4 finalColor = mix(grassColor,landColor,weight);
	finalColor = finalColor * clamp(vec4(normal ,1.0f ),0.0,1.0);
	FragColor = vec4(finalColor.rgb,1.0f);
}