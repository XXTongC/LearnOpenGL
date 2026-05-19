#version 460 core

out vec4 FragColor;
in vec2 uv;

uniform sampler2D srcTex;
uniform int horizontal;

void main()
{
	vec2 texelSize = 1.0 / vec2(textureSize(srcTex, 0));
	vec2 direction = horizontal != 0 ? vec2(texelSize.x, 0.0) : vec2(0.0, texelSize.y);

	vec3 color = texture(srcTex, uv).rgb * 0.227027;
	color += texture(srcTex, uv + direction * 1.384615).rgb * 0.316216;
	color += texture(srcTex, uv - direction * 1.384615).rgb * 0.316216;
	color += texture(srcTex, uv + direction * 3.230769).rgb * 0.070270;
	color += texture(srcTex, uv - direction * 3.230769).rgb * 0.070270;

	FragColor = vec4(color, 1.0);
}
