#include "C:\Dev\LeopphEngine\LeopphEngine\src/rendering/Shader.hpp"
#include <string>
std::string leopph::impl::Shader::s_DirectionalShadowMapFragmentSource{ R"#fileContents#(#version 460 core

void main()
{
	//gl_FragDepth = gl_FragCoord.z;
})#fileContents#" };