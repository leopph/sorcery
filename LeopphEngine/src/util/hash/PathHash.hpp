#pragma once

#include <filesystem>

namespace leopph::impl
{
	struct PathHash
	{
		std::size_t operator()(const std::filesystem::path& path) const;
	};
}