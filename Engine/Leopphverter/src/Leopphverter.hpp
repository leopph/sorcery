#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>


namespace leopph::convert::driver
{
	std::string BuildHeader();
	std::vector<std::string_view> SplitString(std::string_view str, char sep);
	int ParseCommandLine(int argc, char const** argv, std::vector<std::filesystem::path>& filesToConvert, std::vector<std::string>& outputFileNames);
	void ParseInteractive(std::vector<std::filesystem::path>& filesToConvert, std::vector<std::string>& outputFileNames);
}
