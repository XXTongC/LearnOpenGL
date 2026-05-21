#version 460 core

in vec2 uv;

uniform sampler2D albedoMap;
uniform int useAlbedoMap;
uniform float alphaCutoff;

void main()
{
	if (useAlbedoMap == 1 && texture(albedoMap, uv).a < alphaCutoff)
	{
		discard;
	}
}
