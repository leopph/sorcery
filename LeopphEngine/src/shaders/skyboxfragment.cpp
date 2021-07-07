#include "C:\Dev\LeopphEngine\LeopphEngine\src/rendering/shader.h"
#include <string>
std::string leopph::impl::Shader::s_SkyboxFragmentSource{ R"#fileContents#(#version 460 core

in vec3 texCoords;

out vec4 fragColor;

uniform samplerCube skybox;

void main()
{
	fragColor = texture(skybox, texCoords);
})#fileContents#" };