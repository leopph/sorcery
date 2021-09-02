#pragma once

#include "../../data/texturereference.h"

#include <cstddef>
#include <filesystem>

namespace leopph::impl
{
	struct TextureHash
	{
		using is_transparent = void;

		std::size_t operator()(const TextureReference& texture) const;
		std::size_t operator()(const std::filesystem::path& path) const;
		std::size_t operator()(const decltype(TextureReference::id)& id) const;
	};
}