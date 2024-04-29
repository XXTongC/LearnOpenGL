#version 460 core
uniform float time;
uniform float speed;
uniform vec3 uColor;
uniform mat4 transform;
in vec3 aPos; //0
in vec3 aColor;//1
in vec2 aUV;//2
out vec3 color;
out vec3 position;
out vec2 uv;

void main()
{

	gl_Position = transform * vec4(aPos, 1.0f);
	color = aColor;
	uv = aUV;

}