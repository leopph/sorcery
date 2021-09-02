#pragma once

#include "../../data/managed/UniqueResource.hpp"

#include <cstddef>
#include <filesystem>

namespace leopph::impl
{
	struct UniqueResourceHash
	{
		using is_transparent = void;

		std::size_t operator()(const UniqueResource* resource) const;
		std::size_t operator()(const std::filesystem::path& path) const;
	};
}
