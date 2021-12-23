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

			auto operator=(const Texture& other) -> Texture& = delete;
			auto operator=(Texture&& other) -> Texture& = delete;

			const unsigned& Id;
			const std::filesystem::path Path;
			const bool& IsTransparent;

		private:
			unsigned m_ID;
			bool m_IsTransparent;
	};
}
