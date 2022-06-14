#version 430 core

layout (location = 0) in vec3 in_Pos;
layout (location = 3) in mat4 in_ModelMat;

layout (location = 0) out vec3 out_FragPos;

uniform mat4 u_ViewProjMat;


void main()
{
	vec4 worldPos = vec4(in_Pos, 1) * in_ModelMat;
	out_FragPos = vec3(worldPos);
	gl_Position = worldPos * u_ViewProjMat;
}