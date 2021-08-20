#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;
layout (location = 3) in mat4 modelMatrix;
layout (location = 7) in mat4 normalMatrix;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outTexCoords;
layout (location = 2) out vec3 outFragPos;

uniform mat4 viewProjectionMatrix;

void main()
{
    gl_Position = vec4(inPos, 1.0) * modelMatrix * viewProjectionMatrix;
    outNormal = inNormal * mat3(normalMatrix);
    outTexCoords = inTexCoords;
    outFragPos = vec3(vec4(inPos, 1.0) * modelMatrix);
}