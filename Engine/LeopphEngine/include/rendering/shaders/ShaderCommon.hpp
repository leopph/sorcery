#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace leopph
{
	struct ShaderStageSourceFileInfo
	{
		std::filesystem::path absolutePath;
		std::vector<std::string> fileContents;
	};

	struct ShaderProgramSourceFileInfo
	{
		std::optional<ShaderStageSourceFileInfo> vertexInfo;
		std::optional<ShaderStageSourceFileInfo> geometryInfo;
		std::optional<ShaderStageSourceFileInfo> fragmentInfo;
	};

	struct ShaderProgramSourceInfo
	{
		std::optional<std::vector<std::string>> vertex;
		std::optional<std::vector<std::string>> geometry;
		std::optional<std::vector<std::string>> fragment;
	};
}
