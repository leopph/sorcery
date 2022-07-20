#pragma once

#include "Types.hpp"

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
		ShaderStageSourceFileInfo vertexInfo;
		std::optional<ShaderStageSourceFileInfo> geometryInfo;
		std::optional<ShaderStageSourceFileInfo> fragmentInfo;
	};

	struct ShaderProgramSourceInfo
	{
		std::vector<std::string> vertex;
		std::optional<std::vector<std::string>> geometry;
		std::optional<std::vector<std::string>> fragment;
	};

	struct ShaderOptionInfo
	{
		std::string name;
		u8 minValue;
		u8 maxValue;
	};
}