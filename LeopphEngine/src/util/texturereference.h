#pragma once

#include "../rendering/texture.h"

#include <cstddef>

namespace leopph::impl
{
	struct TextureReference
	{
		decltype(Texture::path) path;
		decltype(Texture::id) id;
		mutable std::size_t count;
	};
}