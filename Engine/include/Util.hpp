#pragma once

#include "LeopphApi.hpp"

#include <algorithm>
#include <concepts>
#include <filesystem>
#include <limits>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>



namespace leopph
{
	[[nodiscard]] LEOPPHAPI std::string read_file(std::filesystem::path const& path);
	LEOPPHAPI std::vector<std::string>& read_file_lines(std::filesystem::path const& path, std::vector<std::string>& out);

	LEOPPHAPI void write_file(std::filesystem::path const& path, std::string_view contents);


	LEOPPHAPI void split_lines(std::string_view str, std::vector<std::string>& out);
	LEOPPHAPI void split_words(std::string_view view, std::vector<std::string_view>& out);
	LEOPPHAPI void split_string_delim(std::string_view str, char sep, std::vector<std::string_view>& out);


	template<std::integral To, std::integral From>
	[[nodiscard]] To clamp_cast(From what);
}



template<std::integral To, std::integral From>
To leopph::clamp_cast(From const what)
{
	if constexpr (std::cmp_less_equal(std::numeric_limits<To>::min(), std::numeric_limits<From>::min()))
	{
		if constexpr (std::cmp_greater_equal(std::numeric_limits<To>::max(), std::numeric_limits<From>::max()))
		{
			return static_cast<To>(what);
		}
		else
		{
			return static_cast<To>(std::min<From>(what, std::numeric_limits<To>::max()));
		}
	}
	else
	{
		if constexpr (std::cmp_greater_equal(std::numeric_limits<To>::max(), std::numeric_limits<From>::max()))
		{
			return static_cast<To>(std::max<From>(what, std::numeric_limits<To>::min()));
		}
		else
		{
			return static_cast<To>(std::clamp<From>(what, std::numeric_limits<To>::min(), std::numeric_limits<To>::max()));
		}
	}
}
