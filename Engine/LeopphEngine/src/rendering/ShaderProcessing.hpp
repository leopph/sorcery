#pragma once

#include <filesystem>
#include <string>
#include <vector>


namespace leopph
{
	struct ShaderProgramSourceFileInfo
	{
		std::filesystem::path absolutePath;
		std::vector<std::string> lines;
	};


	using ShaderProgramSourceLines = std::vector<std::string>;

	[[nodiscard]] bool read_shader_files(std::filesystem::path const& filePath, ShaderProgramSourceFileInfo& out);
	void process_shader_includes(ShaderProgramSourceFileInfo sourceFileInfo, ShaderProgramSourceLines& out);
}
