#pragma once

#include <filesystem>

namespace leopph::implementation
{
	// CLASS TO REPRESENT A LOADED TEXTURE
	class Texture
	{
	public:
		enum class TextureType
		{
			DIFFUSE, SPECULAR
		};


		Texture(const std::filesystem::path& path, TextureType type);
		~Texture();

		Texture(const Texture& other);
		Texture(Texture&& other) noexcept;

		Texture& operator=(const Texture& other);
		Texture& operator=(Texture&& other) noexcept;

		bool operator==(const Texture& other) const;
		bool operator==(const std::filesystem::path& path) const;

		unsigned ID() const;
		TextureType Type() const;

	private:
		unsigned m_ID;
		TextureType m_Type;
		std::filesystem::path m_Path;
	};
}