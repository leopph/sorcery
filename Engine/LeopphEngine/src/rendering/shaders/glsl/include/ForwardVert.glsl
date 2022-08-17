//? #version 450 core

#ifndef FORWARD_VERT_GLSL
#define FORWARD_VERT_GLSL

#include "CameraData.glsl"

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexCoords;
layout (location = 3) in mat4 inModelMat;
layout (location = 7) in mat3 inNormalMat;

layout (location = 0) out vec3 outFragPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outTexCoords;

//! #define NUM_DIRLIGHT_SHADOW_CASCADE 1
/*#if NUM_DIRLIGHT_SHADOW_CASCADE
layout (location = 3) out float out_FragPosNdcZ;
#endif*/


void main()
{
    vec4 fragPosWorldSpace = vec4(inPos, 1.0) * inModelMat;
    outNormal = inNormal * inNormalMat;
    outFragPos = fragPosWorldSpace.xyz;
    outTexCoords = inTexCoords;
    gl_Position = fragPosWorldSpace * uViewProjMat;

    /*#if NUM_DIRLIGHT_SHADOW_CASCADE
    out_FragPosNdcZ = gl_Position.z / gl_Position.w;
    #endif*/
}

#endif