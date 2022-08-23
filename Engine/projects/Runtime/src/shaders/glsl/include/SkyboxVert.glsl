//? #version 450 core

#ifndef SKYBOX_VERT_GLSL
#define SKYBOX_VERT_GLSL

layout (location = 0) in vec3 in_Pos;
layout (location = 0) out vec3 out_TexCoords;

uniform mat4 u_ViewProjMat;

void main()
{
	gl_Position = (vec4(in_Pos, 1) * u_ViewProjMat).xyww;
	out_TexCoords = in_Pos;
}

#endif