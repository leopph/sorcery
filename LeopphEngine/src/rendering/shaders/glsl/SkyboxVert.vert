#version 330 core

layout(location = 0) in vec3 inPos;

out vec3 texCoords;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	texCoords = inPos;
	gl_Position = (vec4(inPos, 1) * viewMatrix * projectionMatrix).xyww;
}