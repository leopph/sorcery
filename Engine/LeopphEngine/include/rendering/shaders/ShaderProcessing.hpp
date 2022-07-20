#pragma once

#include "ShaderCommon.hpp"

#include <filesystem>

namespace leopph
{
	[[nodiscard]] auto ReadShaderFiles(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath = {}, std::filesystem::path fragmentShaderPath = {}) -> ShaderProgramSourceFileInfo;

	[[nodiscard]] auto ProcessShaderIncludes(ShaderProgramSourceFileInfo sourceFileInfo) -> ShaderProgramSourceInfo;

	[[nodiscard]] auto ExtractShaderOptions(ShaderProgramSourceInfo& sourceInfo) -> std::vector<ShaderOptionInfo>;	
}
