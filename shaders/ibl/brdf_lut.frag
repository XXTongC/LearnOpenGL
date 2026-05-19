#version 460 core

const float PI = 3.14159265359;
const uint SAMPLE_COUNT = 1024u;

in vec2 uv;
out vec2 FragColor;

float radicalInverseVdc(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10;
}

vec2 hammersley(uint index, uint count)
{
	return vec2(float(index) / float(count), radicalInverseVdc(index));
}

vec3 importanceSampleGGX(vec2 xi, vec3 n, float roughness)
{
	float a = roughness * roughness;
	float phi = 2.0 * PI * xi.x;
	float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (a * a - 1.0) * xi.y));
	float sinTheta = sqrt(max(1.0 - cosTheta * cosTheta, 0.0));

	vec3 h;
	h.x = cos(phi) * sinTheta;
	h.y = sin(phi) * sinTheta;
	h.z = cosTheta;

	vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, n));
	vec3 bitangent = cross(n, tangent);
	return normalize(tangent * h.x + bitangent * h.y + n * h.z);
}

float geometrySchlickGGX(float nDotV, float roughness)
{
	float a = roughness;
	float k = (a * a) / 2.0;
	return nDotV / max(nDotV * (1.0 - k) + k, 0.0001);
}

float geometrySmith(vec3 n, vec3 v, vec3 l, float roughness)
{
	float nDotV = max(dot(n, v), 0.0);
	float nDotL = max(dot(n, l), 0.0);
	return geometrySchlickGGX(nDotV, roughness) * geometrySchlickGGX(nDotL, roughness);
}

vec2 integrateBrdf(float nDotV, float roughness)
{
	vec3 v;
	v.x = sqrt(max(1.0 - nDotV * nDotV, 0.0));
	v.y = 0.0;
	v.z = nDotV;

	float a = 0.0;
	float b = 0.0;
	vec3 n = vec3(0.0, 0.0, 1.0);

	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		vec2 xi = hammersley(i, SAMPLE_COUNT);
		vec3 h = importanceSampleGGX(xi, n, roughness);
		vec3 l = normalize(2.0 * dot(v, h) * h - v);

		float nDotL = max(l.z, 0.0);
		float nDotH = max(h.z, 0.0);
		float vDotH = max(dot(v, h), 0.0);

		if (nDotL > 0.0)
		{
			float geometry = geometrySmith(n, v, l, roughness);
			float geometryVisible = (geometry * vDotH) / max(nDotH * nDotV, 0.0001);
			float fresnel = pow(1.0 - vDotH, 5.0);
			a += (1.0 - fresnel) * geometryVisible;
			b += fresnel * geometryVisible;
		}
	}

	return vec2(a, b) / float(SAMPLE_COUNT);
}

void main()
{
	FragColor = integrateBrdf(uv.x, uv.y);
}
