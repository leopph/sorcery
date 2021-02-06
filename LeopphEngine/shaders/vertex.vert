#version 460 core


layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTextureCoords;


uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 normalMatrix;


out vec3 fragmentPosition;
out vec3 normal;
out vec2 textureCoords;


void main()
{
    gl_Position = vec4(inPosition, 1.0) * model * view * proj;
    fragmentPosition = vec3(vec4(inPosition, 1.0f) * model);
    normal = inNormal * mat3(normalMatrix);
    textureCoords = inTextureCoords;
}