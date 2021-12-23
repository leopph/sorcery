#pragma once

#include <filesystem>


namespace leopph::internal
{
	struct PathHash
	{
		auto operator()(const std::filesystem::path& path) const -> std::size_t;
	};
}
