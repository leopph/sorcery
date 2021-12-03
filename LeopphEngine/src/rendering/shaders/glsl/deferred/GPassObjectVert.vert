#version 430 core

layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;
#ifdef INSTANCED
layout (location = 3) in mat4 in_ModelMat;
layout (location = 7) in mat4 in_NormalMat;
#endif

layout (location = 0) out vec3 out_FragPos;
layout (location = 1) out vec3 out_Normal;
layout (location = 2) out vec2 out_TexCoords;
layout (location = 3) out float out_ClipSpaceZ;

layout (location = 0) uniform mat4 u_ViewProjMat;
#ifndef INSTANCED
uniform mat4 u_ModelMat;
uniform mat4 u_NormalMat;
#endif


void main()
{
#ifdef INSTANCED
    vec4 fragPosWorldSpace = vec4(in_Pos, 1.0) * in_ModelMat;
    out_Normal = in_Normal * mat3(in_NormalMat);
#else
    vec4 fragPosWorldSpace = vec4(in_Pos, 1.0) * u_ModelMat;
    out_Normal = in_Normal * mat3(u_NormalMat);
#endif

    out_FragPos = fragPosWorldSpace.xyz;
    out_TexCoords = in_TexCoords;

    gl_Position = fragPosWorldSpace * u_ViewProjMat;
    out_ClipSpaceZ = gl_Position.z;
}