#pragma once

#include <filesystem>
#include <memory>


namespace leopph
{
	class Texture : public std::enable_shared_from_this<Texture>
	{
	public:
		Texture(std::filesystem::path path);

		Texture(const Texture& other) = delete;
		Texture(Texture&& other) = delete;

		~Texture();

		Texture& operator=(const Texture& other) = delete;
		Texture& operator=(Texture&& other) = delete;

		const unsigned& Id;
		const std::filesystem::path Path;
		const bool& IsTransparent;


	private:
		unsigned m_ID;
		bool m_IsTransparent;
	};
}