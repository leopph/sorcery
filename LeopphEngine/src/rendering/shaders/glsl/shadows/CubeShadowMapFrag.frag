#version 460 core

layout (location = 0) in vec3 in_FragPos;

layout (location = 6) uniform vec3 u_LightPos;
layout (location = 7) uniform float u_FarPlane;


void main()
{
	float dist = length(in_FragPos.xyz - u_LightPos);
	dist /= u_FarPlane;
	gl_FragDepth = dist;
}