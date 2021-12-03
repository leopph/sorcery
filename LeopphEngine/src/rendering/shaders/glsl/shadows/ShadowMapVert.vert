#version 330 core

layout (location = 0) in vec3 in_Pos;
#ifdef INSTANCED
layout (location = 3) in mat4 in_ModelMat;
#endif

#ifndef INSTANCED
uniform mat4 u_ModelMat;
#endif
uniform mat4 u_WorldToClipMat;


void main()
{
#ifdef INSTANCED
	gl_Position = vec4(in_Pos, 1.0) * in_ModelMat * u_WorldToClipMat;
#else
	gl_Position = vec4(in_Pos, 1.0) * u_ModelMat * u_WorldToClipMat;
#endif
}