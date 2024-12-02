#version 460 core

in vec2 aPos; //0
in vec2 aUV;//2

out vec4 color;
out vec2 uv;

out vec3 worldPosition;


void main()
{	
	gl_Position = vec4(aPos.x,aPos.y,0.0f,1.0f);
	uv = aUV;
}