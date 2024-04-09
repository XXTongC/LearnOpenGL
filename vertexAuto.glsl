#version 460 core
uniform float time;
uniform float speed;
uniform vec3 uColor;
in vec3 aPos;
in vec3 aColor;
out vec3 color;
out vec3 position;
void main()
{
	gl_Position = vec4(aPos.x + sin(time * speed) / 2, aPos.y, aPos.z, 1.0);
	color = uColor * ((cos(time) + 1) / 2);
	position = aPos;
}