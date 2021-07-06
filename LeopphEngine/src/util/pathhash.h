#pragma once

#include <filesystem>

namespace std
{
	template<>
	struct hash<std::filesystem::path>
	{
		std::size_t operator()(const std::filesystem::path& path) const noexcept
		{
			return filesystem::hash_value(path);
		}
	};
}