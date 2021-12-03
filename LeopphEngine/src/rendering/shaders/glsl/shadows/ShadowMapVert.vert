#version 330 core

layout (location = 0) in vec3 in_Pos;

uniform mat4 u_ModelMat;
uniform mat4 u_WorldToClipMat;


void main()
{
	gl_Position = vec4(in_Pos, 1.0) * u_ModelMat * u_WorldToClipMat;
}