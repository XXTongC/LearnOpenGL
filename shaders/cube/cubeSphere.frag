#version 460 core

out vec4 FragColor;
in vec3 uvw;
uniform sampler2D sphericalSampler;

vec2 uvwToUV(vec3 uvwN);

const float PI = 3.14159265358979323;

void main()
{
	vec3 uvwN = normalize(uvw);
	FragColor = texture(sphericalSampler,uvwToUV(uvwN));
	//FragColor = vec4(1.0f,1.0f,1.0f,1.0f);
}

vec2 uvwToUV(vec3 uvwN)
{
	float phi = asin(uvwN.y);
	float theta = atan(uvwN.z,uvwN.x);
	float u = theta / (2 * PI) + 0.5;
	float v = phi / PI + 0.5;
	return vec2(u,v);
}
