#pragma once

#include "../data/managed/UniqueResource.hpp"

#include <filesystem>


namespace leopph::impl
{
	class TextureResource : public UniqueResource
	{
	public:
		TextureResource(const std::filesystem::path& path);

		TextureResource(const TextureResource& other) = delete;
		TextureResource(TextureResource&& other) = delete;

		TextureResource& operator=(const TextureResource& other) = delete;
		TextureResource& operator=(TextureResource&& other) = delete;

		~TextureResource();

		const unsigned& Id;
		const std::filesystem::path Path;
		const bool& IsTransparent;

	private:
		unsigned m_ID;
		bool m_IsTransparent;
	};
}