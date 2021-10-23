#version 460 core

layout (location = 0) in vec3 in_Position;
layout (location = 3) in mat4 in_ModelMatrix;

uniform mat4 u_LightWorldToClipMatrix;


void main()
{
	gl_Position = vec4(in_Position, 1.0) * in_ModelMatrix * u_LightWorldToClipMatrix;
}