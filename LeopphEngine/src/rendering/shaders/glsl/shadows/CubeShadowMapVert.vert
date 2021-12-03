#version 330 core

layout (location = 0) in vec3 in_Pos;
#ifdef INSTANCED
layout (location = 3) in mat4 in_ModelMat;
#endif

#ifndef INSTANCED
uniform mat4 u_ModelMat;
#endif


void main()
{
#ifdef INSTANCED
	gl_Position = vec4(in_Pos, 1) * in_ModelMat;
#else
	gl_Position = vec4(in_Pos, 1) * u_ModelMat;
#endif
}