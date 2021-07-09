#include "C:\Dev\LeopphEngine\LeopphEngine\src/rendering/shader.h"
#include <string>
std::string leopph::impl::Shader::s_VertexSource{ R"#fileContents#(#version 460 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTextureCoords;
layout (location = 3) in mat4 modelViewMatrix;
layout (location = 7) in mat4 normalMatrix;

uniform mat4 projectionMatrix;

out vec3 normal;
out vec2 textureCoords;
out vec3 fragPos;

void main()
{
    gl_Position = vec4(inPosition, 1.0) * modelViewMatrix * projectionMatrix;
    normal = inNormal * mat3(normalMatrix);
    textureCoords = inTextureCoords;
    fragPos = vec3(vec4(inPosition, 1.0) * modelViewMatrix);
})#fileContents#" };