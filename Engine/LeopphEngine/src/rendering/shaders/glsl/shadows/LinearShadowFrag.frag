#version 430 core

layout (location = 0) in vec3 in_FragPos;

layout (location = 0) out float out_Dist;

uniform vec3 u_LightPos;


void main()
{
	out_Dist = length(in_FragPos - u_LightPos);
}