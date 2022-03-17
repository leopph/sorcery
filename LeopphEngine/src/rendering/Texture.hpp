#pragma once

#include <filesystem>
#include <memory>


namespace leopph
{
	class Texture : public std::enable_shared_from_this<Texture>
	{
		public:
			explicit Texture(std::filesystem::path path);

			Texture(const Texture& other) = delete;
			auto operator=(const Texture& other) -> Texture& = delete;

			Texture(Texture&& other) = delete;
			auto operator=(Texture&& other) -> Texture& = delete;

			~Texture();

			[[nodiscard]] constexpr auto Id() const;
			[[nodiscard]] constexpr auto IsTransparent() const;
			[[nodiscard]] constexpr auto Path() const -> auto&;
			[[nodiscard]] constexpr auto Width() const noexcept;
			[[nodiscard]] constexpr auto Height() const noexcept;

		private:
			unsigned m_TexName;
			bool m_IsTransparent;
			std::filesystem::path m_Path;
			int m_Width;
			int m_Height;
	};


	constexpr auto Texture::Id() const
	{
		return m_TexName;
	}


	constexpr auto Texture::IsTransparent() const
	{
		return m_IsTransparent;
	}


	constexpr auto Texture::Path() const -> auto&
	{
		return m_Path;
	}


	constexpr auto Texture::Width() const noexcept
	{
		return m_Width;
	}


	constexpr auto Texture::Height() const noexcept
	{
		return m_Height;
	}
}
