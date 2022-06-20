#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>


namespace leopph::convert::driver
{
	auto BuildHeader() -> std::string;
	auto SplitString(std::string_view const str, char const sep) -> std::vector<std::string_view>;
	auto ParseCommandLine(int argc, char const** argv, std::vector<std::filesystem::path>& filesToConvert, std::vector<std::string_view>& outputFileNames) -> int;
}
