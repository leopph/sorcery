#pragma once

#include <filesystem>
#include <string>
#include <vector>


namespace leopph::convert::driver
{
	int parse_command_line(int argc, char const** argv, std::vector<std::filesystem::path>& filesToConvert, std::vector<std::string>& outputFileNames);
	void parse_interactive(std::vector<std::filesystem::path>& filesToConvert, std::vector<std::string>& outputFileNames);
}
