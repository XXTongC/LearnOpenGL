#version 460 core

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in vec3 aPos; //0
out vec3 uvw;
void main()
{	
	vec4 transformPosition = vec4(aPos,1.0);
	transformPosition = modelMatrix * transformPosition;
	gl_Position = projectionMatrix * viewMatrix * transformPosition;
	//使得z坐标深度恒为一
	gl_Position = gl_Position.xyww;
	uvw = aPos;
}