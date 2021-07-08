#include "C:\Dev\LeopphEngine\LeopphEngine\src/rendering/shader.h"
#include <string>
std::string leopph::impl::Shader::s_SkyboxVertexSource{ R"#fileContents#(#version 460 core

layout(location = 0) in vec3 inPos;

out vec3 texCoords;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
	texCoords = inPos;
	gl_Position = (vec4(inPos, 1) * viewMatrix * projectionMatrix).xyww;
})#fileContents#" };