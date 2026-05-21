#version 460 core

in vec2 uv;
in vec3 worldPosition;

uniform sampler2D albedoMap;
uniform int useAlbedoMap;
uniform float alphaCutoff;
uniform vec3 lightPos;
uniform float far_plane;

void main()
{
	if (useAlbedoMap == 1 && texture(albedoMap, uv).a < alphaCutoff)
	{
		discard;
	}

	float lightDistance = length(worldPosition - lightPos);
	gl_FragDepth = lightDistance / (1.414 * far_plane);
}
