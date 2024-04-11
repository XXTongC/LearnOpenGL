#version 460 core
uniform float time;
uniform sampler2D samplerGrass;
uniform sampler2D samplerLand;
uniform sampler2D samplerNoise;
in vec3 color;
in vec2 uv;//2
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
	float weight = noiseColor.r * sin(time/10);
	vec4 finalColor = mix(grassColor,landColor,weight);
	
	FragColor = vec4(finalColor.rgb,1.0f);
}