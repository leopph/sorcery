#pragma once

#include "LeopphApi.hpp"

#include <algorithm>
#include <concepts>
#include <filesystem>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>



namespace leopph
{
	[[nodiscard]] auto LEOPPHAPI ReadFile(std::filesystem::path const& path) -> std::string;
	auto LEOPPHAPI ReadFileLines(std::filesystem::path const& path, std::vector<std::string>& out) -> std::vector<std::string>&;
	auto LEOPPHAPI WriteFile(std::filesystem::path const& path, std::string_view contents) -> void;

	auto LEOPPHAPI SplitLines(std::string_view str, std::vector<std::string>& out) -> void;
	auto LEOPPHAPI SplitWords(std::string_view const view, std::vector<std::string_view>& out) -> void;

	template<std::integral To, std::integral From>
	[[nodiscard]] auto ClampCast(From const from) -> To;
}



template<std::integral To, std::integral From>
auto leopph::ClampCast(From const from) -> To
{
	if constexpr (std::cmp_less_equal(std::numeric_limits<To>::min(), std::numeric_limits<From>::min()))
	{
		if constexpr (std::cmp_greater_equal(std::numeric_limits<To>::max(), std::numeric_limits<From>::max()))
		{
			return static_cast<To>(from);
		}
		else
		{
			return static_cast<To>(std::min<From>(from, std::numeric_limits<To>::max()));
		}
	}
	else
	{
		if constexpr (std::cmp_greater_equal(std::numeric_limits<To>::max(), std::numeric_limits<From>::max()))
		{
			return static_cast<To>(std::max<From>(from, std::numeric_limits<To>::min()));
		}
		else
		{
			return static_cast<To>(std::clamp<From>(from, std::numeric_limits<To>::min(), std::numeric_limits<To>::max()));
		}
	}
}
