#version 460 core
uniform float time;
uniform float speed;
uniform vec3 uColor;
uniform mat4 transform;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
in vec3 aPos; //0
in vec4 aColor;//1
in vec2 aUV;//2
out vec4 color;
out vec2 uv;




void main()
{	
	vec4 position = vec4(aPos, 1.0f);
	position = projectionMatrix * viewMatrix * transform * position;
	gl_Position = position;
	color = aColor;
	uv = aUV;
}