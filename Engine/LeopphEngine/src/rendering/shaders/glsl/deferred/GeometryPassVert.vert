#version 430 core

layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;
layout (location = 3) in mat4 in_ModelMat;
layout (location = 7) in mat4 in_NormalMat;

layout (location = 0) out vec3 out_Normal;
layout (location = 1) out vec2 out_TexCoords;

uniform mat4 u_ViewProjMat;


void main()
{
    gl_Position = vec4(in_Pos, 1.0) * in_ModelMat * u_ViewProjMat;
    out_Normal = in_Normal * mat3(in_NormalMat);
    out_TexCoords = in_TexCoords;
}