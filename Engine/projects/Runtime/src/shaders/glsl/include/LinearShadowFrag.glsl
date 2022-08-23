//? #version 450 core

#ifndef LINEAR_SHADOW_FRAG_GLSL
#define LINEAR_SHADOW_FRAG_GLSL

layout (location = 0) in vec3 in_FragPos;
layout (location = 0) out float out_Dist;

uniform vec3 u_LightPos;


void main()
{
	out_Dist = length(in_FragPos - u_LightPos);
}

#endif