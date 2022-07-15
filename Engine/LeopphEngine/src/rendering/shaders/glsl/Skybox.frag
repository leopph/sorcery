#version 410 core

layout(location = 0) in vec3 in_TexCoords;
layout(location = 0) out vec3 out_FragColor;

uniform samplerCube u_CubeMap;


void main()
{
	out_FragColor = texture(u_CubeMap, in_TexCoords).rgb;
}