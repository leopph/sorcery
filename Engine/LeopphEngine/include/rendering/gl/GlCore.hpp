#pragma once

#include "rendering/shaders/ShaderType.hpp"

#include <glad/gl.h>

#include <vector>


namespace leopph::internal::opengl
{
	// Initializes the OpenGL functions
	// Sets callbacks
	auto Init() -> void;

	// Returns a list of supported binary formats
	auto ShaderBinaryFormats() -> std::vector<GLint>;

	// Returns the mapping of an abstract ShaderType to an OpenGL one
	auto TranslateShaderType(ShaderType type) -> decltype(GL_VERTEX_SHADER);
	// Returns the mapping of an OpenGL shader type to a ShaderType instance
	auto TranslateShaderType(decltype(GL_VERTEX_SHADER) type) -> ShaderType;
}
