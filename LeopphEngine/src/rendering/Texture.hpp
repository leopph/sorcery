#pragma once

#include "../data/managed/ResourceHandle.hpp"
#include "TextureResource.hpp"

#include <filesystem>


namespace leopph::impl
{
	class Texture : public ResourceHandle<TextureResource>
	{
	public:
		explicit Texture(const std::filesystem::path& path);
		Texture(const Texture& other);
		Texture(Texture&& other);

		Texture& operator=(const Texture& other);
		Texture& operator=(Texture&& other);

		~Texture() = default;

		const decltype(TextureResource::Id)& Id() const;
		const decltype(TextureResource::IsTransparent)& IsTransparent() const;
		const decltype(TextureResource::Path)& Path() const;

	};
}