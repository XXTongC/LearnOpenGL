#version 460 core

in vec3 aPos;

uniform mat4 lightMatrix;  // lightProjection * lightView
uniform mat4 modelMatrix;

void main()
{
    gl_Position = lightMatrix * modelMatrix * vec4(aPos,1.0);
    
}
