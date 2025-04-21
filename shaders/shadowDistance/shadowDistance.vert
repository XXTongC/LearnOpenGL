#version 460 core
in vec3 aPos;

uniform mat4 modelMatrix;
uniform mat4 lightSpaceMatrix;
out vec3 worldPosition;
void main()
{
    worldPosition = (modelMatrix * vec4(aPos, 1.0)).xyz;
    gl_Position = lightSpaceMatrix * modelMatrix * vec4(aPos, 1.0);
   
}

