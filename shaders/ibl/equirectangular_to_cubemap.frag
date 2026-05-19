#version 460 core

in vec3 localPosition;
out vec4 FragColor;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.15915494309, 0.31830988618);

vec2 sampleSphericalMap(vec3 direction)
{
	vec2 uv = vec2(atan(direction.z, direction.x), asin(direction.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

void main()
{
	vec3 direction = normalize(localPosition);
	vec2 uv = sampleSphericalMap(direction);
	vec3 color = texture(equirectangularMap, uv).rgb;
	FragColor = vec4(color, 1.0);
}
