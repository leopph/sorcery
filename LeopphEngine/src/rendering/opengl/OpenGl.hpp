#pragma once

#include "../../rendering/shaders/ShaderType.hpp"

#include <vector>


namespace leopph::internal::opengl
{
	// Initializes the OpenGL functions
	// Sets callbacks
	auto Init() -> bool;

	// Returns a list of supported binary formats
	auto ShaderBinaryFormats() -> std::vector<int>;

	// Returns the mapping of an abstract ShaderType to an OpenGL one
	auto TranslateShaderType(ShaderType type) -> int;
	// Returns the mapping of an OpenGL shader type to a ShaderType instance
	auto TranslateShaderType(int type) -> ShaderType;
}
