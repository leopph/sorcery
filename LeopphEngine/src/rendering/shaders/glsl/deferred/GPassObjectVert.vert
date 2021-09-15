#version 460 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;
layout (location = 3) in mat4 inModelMatrix;
layout (location = 7) in mat4 inNormalMatrix;

layout (location = 0) out vec3 outFragPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outTexCoords;
layout (location = 3) out float outClipSpaceZ;

layout (location = 0) uniform mat4 u_ViewProjectionMatrix;


void main()
{
    vec4 fragPosWorldSpace = vec4(inPos, 1.0) * inModelMatrix;

    outFragPos = fragPosWorldSpace.xyz;
    outNormal = inNormal * mat3(inNormalMatrix);
    outTexCoords = inTexCoords;

    gl_Position = fragPosWorldSpace * u_ViewProjectionMatrix;
    outClipSpaceZ = gl_Position.z;
}