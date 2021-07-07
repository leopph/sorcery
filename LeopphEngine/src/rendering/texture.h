#pragma once

#include <filesystem>

namespace leopph::impl
{
	struct TextureReference;

	struct Texture
	{
		const unsigned& id;
		const std::filesystem::path path;
		const bool& isTransparent;


		Texture(const std::filesystem::path& path);
		Texture(const TextureReference& reference);
		~Texture();

		Texture(const Texture& other);
		Texture& operator=(const Texture& other) = delete;

		bool operator==(const Texture& other) const;
		bool operator==(const std::filesystem::path& other) const;

	private:
		unsigned m_ID;
		bool m_IsTransparent;
	};
}