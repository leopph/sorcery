#include "C:\Dev\LeopphEngine\LeopphEngine\src/rendering/Shader.hpp"
#include <string>
std::string leopph::impl::Shader::s_DebugVertexSource{ R"#fileContents#(#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
})#fileContents#" };