#version 410 core

#ifdef EXISTS_DIRLIGHT
#ifdef DIRLIGHT_SHADOW
#define MAX_DIR_LIGHT_CASCADE_COUNT 3
#endif
#endif

layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;
layout (location = 3) in mat4 in_ModelMatrix;
layout (location = 7) in mat4 in_NormalMatrix;

layout (location = 2) out vec3 out_FragPos;
layout (location = 0) out vec3 out_Normal;
layout (location = 1) out vec2 out_TexCoords;

uniform mat4 u_ViewProjMat;

#ifdef EXISTS_DIRLIGHT
#ifdef DIRLIGHT_SHADOW
layout (location = 3) out float out_ClipPosZ;
#endif
#endif


void main()
{
    vec4 fragPosWorldSpace = vec4(in_Pos, 1.0) * in_ModelMatrix;

    out_FragPos = fragPosWorldSpace.xyz;
    out_Normal = in_Normal * mat3(in_NormalMatrix);
    out_TexCoords = in_TexCoords;
    gl_Position = fragPosWorldSpace * u_ViewProjMat;

    #ifdef EXISTS_DIRLIGHT
    #ifdef DIRLIGHT_SHADOW
    out_ClipPosZ = gl_Position.z;
    #endif
    #endif
}