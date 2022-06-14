#version 410 core

layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;
layout (location = 3) in mat4 in_ModelMat;
layout (location = 7) in mat4 in_NormalMat;

layout (location = 0) out vec3 out_FragPos;
layout (location = 1) out vec3 out_Normal;
layout (location = 2) out vec2 out_TexCoords;

uniform mat4 u_ViewProjMat;

#if EXISTS_DIRLIGHT && DIRLIGHT_SHADOW
layout (location = 3) out float out_NdcPosZ;
#endif


void main()
{
    vec4 fragPosWorldSpace = vec4(in_Pos, 1.0) * in_ModelMat;
    out_Normal = in_Normal * mat3(in_NormalMat);

    out_FragPos = fragPosWorldSpace.xyz;
    out_TexCoords = in_TexCoords;
    gl_Position = fragPosWorldSpace * u_ViewProjMat;

    #if EXISTS_DIRLIGHT && DIRLIGHT_SHADOW
    out_ClipPosZ = gl_Position.z;
    #endif
}