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

			~Texture() noexcept;

			// Provides ordering based on path.
			[[nodiscard]] inline
			auto operator<=>(const Texture& other) const;

			// Equality based on path.
			[[nodiscard]] inline
			auto operator==(const Texture& other) const -> bool;

			[[nodiscard]] constexpr
			auto Id() const;

			[[nodiscard]] constexpr
			auto IsTransparent() const;

			[[nodiscard]] constexpr
			auto Path() const -> auto&;

			[[nodiscard]] constexpr
			auto Width() const noexcept;

			[[nodiscard]] constexpr
			auto Height() const noexcept;

		private:
			unsigned m_TexName;
			bool m_IsTransparent;
			std::filesystem::path m_Path;
			int m_Width;
			int m_Height;
	};


	inline auto Texture::operator<=>(const Texture& other) const
	{
		return m_Path <=> other.m_Path;
	}


	inline auto Texture::operator==(const Texture& other) const -> bool
	{
		return m_Path == other.m_Path;
	}


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


	// Provides ordering with paths.
	[[nodiscard]] inline
	auto operator<=>(const Texture& tex, const std::filesystem::path& path)
	{
		return tex.Path() <=> path;
	}


	// Provides ordering with paths.
	[[nodiscard]] inline
	auto operator<=>(const std::filesystem::path& path, const Texture& tex)
	{
		return path <=> tex.Path();
	}


	// Provides equality with paths.
	[[nodiscard]] inline
	auto operator==(const Texture& tex, const std::filesystem::path& path) -> bool
	{
		return tex.Path() == path;
	}


	// Provides equality with paths.
	[[nodiscard]] inline
	auto operator==(const std::filesystem::path& path, const Texture& tex) -> bool
	{
		return path == tex.Path();
	}
}
