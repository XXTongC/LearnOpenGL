#version 460 core

const float PI = 3.14159265359;

in vec3 localPosition;
out vec4 FragColor;

uniform samplerCube environmentMap;

void main()
{
	vec3 n = normalize(localPosition);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, n));
	up = normalize(cross(n, right));

	vec3 irradiance = vec3(0.0);
	float sampleDelta = 0.025;
	float sampleCount = 0.0;

	for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			vec3 sampleVector = tangentSample.x * right + tangentSample.y * up + tangentSample.z * n;
			irradiance += texture(environmentMap, sampleVector).rgb * cos(theta) * sin(theta);
			sampleCount += 1.0;
		}
	}

	irradiance = PI * irradiance * (1.0 / max(sampleCount, 1.0));
	FragColor = vec4(irradiance, 1.0);
}
