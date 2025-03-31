#version 460 core
uniform float time;
uniform float speed;
uniform vec3 uColor;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform mat4 lightViewMatrix;
uniform mat4 lightMatrix;   //lightProject * lightView


in vec3 aPos; //0
in vec4 aColor;//1
in vec2 aUV;//2
in vec3 aNormal;//3
out vec4 color;
out vec2 uv;
out vec3 normal;
out vec3 worldPosition;
out vec4 lightSpaceClipCoord;    //光源空间内的剪裁空间坐标
out vec3 lightSpacePosition;
/* 如果在shader计算中需要一些不变量，最好在shader外计算好再传入
 * shader中，否则每个像素都进行这样计算将是十分消耗性能的
*/
void main()
{	
	//将输入的顶点位置转换为齐次坐标（三维--->四维）
	vec4 transformPosition = vec4(aPos,1.0);
	//中间变量transformPosition用于计算其他以其为过程值的变量
	transformPosition = modelMatrix * transformPosition;

	worldPosition = transformPosition.xyz;
	gl_Position = projectionMatrix * viewMatrix * transformPosition;
	//gl_Position = position;
	color = aColor;
	uv = aUV;
	normal = normalMatrix * normalize(aNormal);

	lightSpacePosition = (lightViewMatrix * transformPosition).xyz;
    lightSpaceClipCoord = lightMatrix * transformPosition;
}