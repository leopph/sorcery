#version 450 core

layout(location = 0) in vec3 in_Pos;
layout (location = 1) in vec2 in_TexCoords;

layout (location = 0) out vec2 out_TexCoords;


void main()
{
	gl_Position = vec4(in_Pos, 1);
	out_TexCoords = in_TexCoords;
}