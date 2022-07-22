#pragma once

#include "ShaderCommon.hpp"

#include <filesystem>


namespace leopph
{
	[[nodiscard]] ShaderProgramSourceFileInfo ReadShaderFiles(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath = {}, std::filesystem::path fragmentShaderPath = {});

	[[nodiscard]] ShaderProgramSourceInfo ProcessShaderIncludes(ShaderProgramSourceFileInfo sourceFileInfo);
}
