#version 460 core
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform mat4 directionalLightSpaceMatrix; // 添加这个uniform
in vec3 aPos;
in vec4 aColor;
in vec2 aUV;
in vec3 aNormal;

out vec4 color;
out vec2 uv;
out vec3 normal;

out vec3 worldPosition;
out vec4 directionalLightSpacePos; // 添加这个输出
void main()
{	
    vec4 worldPos = modelMatrix * vec4(aPos, 1.0);
    worldPosition = worldPos.xyz;
    
    gl_Position = projectionMatrix * viewMatrix * worldPos;
    color = aColor;
    uv = aUV;
    normal = normalMatrix * normalize(aNormal);
    directionalLightSpacePos = directionalLightSpaceMatrix * worldPos; // 计算定向光的光空间位置
}