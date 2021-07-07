#pragma once

#include "../rendering/texture.h"

#include <cstddef>
#include <type_traits>

namespace leopph::impl
{
	struct TextureReference
	{
		std::remove_reference<decltype(Texture::path)>::type path;
		std::remove_reference<decltype(Texture::id)>::type id;
		std::remove_reference<decltype(Texture::isTransparent)>::type isTransparent;
		mutable std::size_t count;
	};
}