#version 330 core

layout (location = 0) in vec3 in_Pos;
layout (location = 3) in mat4 in_ModelMat;

uniform mat4 u_ViewProjMat;


void main()
{
	gl_Position = vec4(in_Pos, 1.0) * in_ModelMat * u_ViewProjMat;
}