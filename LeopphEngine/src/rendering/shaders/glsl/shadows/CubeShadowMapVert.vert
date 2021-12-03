#version 330 core

layout (location = 0) in vec3 in_Pos;

uniform mat4 u_ModelMat;


void main()
{
	gl_Position = vec4(in_Pos, 1) * u_ModelMat;
}