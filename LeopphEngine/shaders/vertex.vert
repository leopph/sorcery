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
    gl_Position = proj * view * model * vec4(inPosition, 1.0);
    fragmentPosition = vec3(model * vec4(inPosition, 1.0f));
    normal = mat3(normalMatrix) * inNormal;
    textureCoords = inTextureCoords;
}