#version 410 core

layout (location = 0) in vec3 in_Pos;

layout (location = 0) out vec3 out_TexCoords;;

uniform mat4 u_ViewProjMat;


void main()
{
	out_TexCoords = in_Pos;
	gl_Position = (vec4(in_Pos, 1) * u_ViewProjMat).xyww;
}