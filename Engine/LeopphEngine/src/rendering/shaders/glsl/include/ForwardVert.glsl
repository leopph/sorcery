//? #version 450 core

#ifndef FORWARD_VERT_GLSL
#define FORWARD_VERT_GLSL

#include "TransformBuffer.glsl"
#include "Lighting.glsl"

layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;
layout (location = 3) in mat4 in_ModelMat;
layout (location = 7) in mat4 in_NormalMat;

layout (location = 0) out vec3 out_FragPos;
layout (location = 1) out vec3 out_Normal;
layout (location = 2) out vec2 out_TexCoords;

#if NUM_DIRLIGHT_SHADOW_CASCADE
layout (location = 3) out float out_FragPosNdcZ;
#endif


void main()
{
    vec4 fragPosWorldSpace = vec4(in_Pos, 1.0) * in_ModelMat;
    out_Normal = in_Normal * mat3(in_NormalMat);
    out_FragPos = fragPosWorldSpace.xyz;
    out_TexCoords = in_TexCoords;
    gl_Position = fragPosWorldSpace * u_ViewProjMat;

    #if NUM_DIRLIGHT_SHADOW_CASCADE
    out_FragPosNdcZ = gl_Position.z / gl_Position.w;
    #endif
}

#endif