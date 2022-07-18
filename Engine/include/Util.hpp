#pragma once

#include "LeopphApi.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace leopph
{
	[[nodiscard]] auto LEOPPHAPI ReadFile(std::filesystem::path const& path) -> std::string;

	auto LEOPPHAPI WriteFile(std::filesystem::path const& path, std::string_view contents) -> void;

	auto LEOPPHAPI SplitLines(std::string_view str, std::vector<std::string>& out) -> void;
}
