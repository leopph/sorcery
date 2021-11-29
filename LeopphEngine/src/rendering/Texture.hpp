#pragma once

#include <filesystem>


namespace leopph::impl
{
	class TextureImpl;


	class Texture
	{
	public:
		explicit Texture(std::filesystem::path path);
		Texture(const Texture& other);
		Texture(Texture&& other);

		~Texture();

		Texture& operator=(const Texture& other);
		Texture& operator=(Texture&& other);

		const unsigned& Id() const;
		const bool& IsTransparent() const;
		const std::filesystem::path& Path() const;


	private:
		void Deinit();

		TextureImpl* m_Impl;
	};
}