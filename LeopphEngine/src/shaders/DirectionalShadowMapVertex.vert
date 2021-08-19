#version 460 core
layout (location = 0) in vec3 inPosition;
layout (location = 1) in mat4 modelMatrix;

uniform mat4 lightSpaceMatrix;

void main()
{
	gl_Position = vec4(inPosition, 1.0) * modelMatrix * lightSpaceMatrix;
}