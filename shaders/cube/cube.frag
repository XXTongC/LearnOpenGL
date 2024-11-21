#version 460 core

out vec4 FragColor;
in vec3 uvw;
uniform samplerCube cubeSampler;
void main()
{
	FragColor = texture(cubeSampler,uvw);
}
