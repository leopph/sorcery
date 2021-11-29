#pragma once

#include <filesystem>


namespace leopph::impl
{
	class TextureImpl
	{
	public:
		TextureImpl(std::filesystem::path path);

		TextureImpl(const TextureImpl& other) = delete;
		TextureImpl(TextureImpl&& other) = delete;

		~TextureImpl();

		TextureImpl& operator=(const TextureImpl& other) = delete;
		TextureImpl& operator=(TextureImpl&& other) = delete;

		const unsigned& Id;
		const std::filesystem::path Path;
		const bool& IsTransparent;


	private:
		unsigned m_ID;
		bool m_IsTransparent;
	};
}