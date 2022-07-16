#pragma once

#include <filesystem>
#include <string>

namespace leopph::internal
{
	struct ShaderSourceFileInfo
	{
		std::filesystem::path absolutePath;
		std::string content;
	};

	auto ProcessShader(ShaderSourceFileInfo const& fileInfo) -> std::string;
}
