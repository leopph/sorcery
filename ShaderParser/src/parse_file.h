#pragma once

#include <filesystem>
#include <string>

namespace parser
{
	std::string ParseFile(std::filesystem::path path) noexcept;
}