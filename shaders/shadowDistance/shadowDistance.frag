#version 460 core
in vec3 worldPosition;
uniform vec3 lightPos;
uniform float far_plane;
out vec4 FragColor;
void main()
{
    // 获取片段到光源的距离
    float lightDistance = length(worldPosition - lightPos);
    
    // 将距离标准化到 [0,1] 范围
    lightDistance = lightDistance /(1.414 * far_plane);
    
    // 将标准化的深度写入深度缓冲
    gl_FragDepth = lightDistance;

}