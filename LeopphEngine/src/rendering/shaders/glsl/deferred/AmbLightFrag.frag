#version 460 core

layout (location = 0) in vec2 in_TexCoords;

layout (location = 0) out vec4 out_FragColor;

layout (location = 0) uniform vec3 u_AmbientLight;
layout (location = 1) uniform sampler2D u_AmbientMap;


void main()
{
	vec3 color = texture(u_AmbientMap, in_TexCoords).rgb * u_AmbientLight;
	out_FragColor = vec4(color, 1);
}