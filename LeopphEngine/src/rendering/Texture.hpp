#pragma once

#include <filesystem>
#include <memory>
#include <span>


namespace leopph
{
	class Texture : public std::enable_shared_from_this<Texture>
	{
		public:
			explicit Texture(std::filesystem::path path);

			Texture(Texture const& other) = delete;
			auto operator=(Texture const& other) -> Texture& = delete;

			Texture(Texture&& other) = delete;
			auto operator=(Texture&& other) -> Texture& = delete;

			~Texture() noexcept;

			// Provides ordering based on path.
			[[nodiscard]] inline
			auto operator<=>(Texture const& other) const;

			// Equality based on path.
			[[nodiscard]] inline
			auto operator==(Texture const& other) const -> bool;

			[[nodiscard]] constexpr
			auto Id() const;

			// A Texture is semi-transparent if it contains pixels
			// with an alpha value less than 1.
			[[nodiscard]] constexpr
			auto IsSemiTransparent() const;

			// A Texture is transparent if all of its pixels have
			// an alpha values less than 1.
			[[nodiscard]]
			auto IsTransparent() const -> bool;

			[[nodiscard]] constexpr
			auto Path() const -> auto&;

			[[nodiscard]] constexpr
			auto Width() const noexcept;

			[[nodiscard]] constexpr
			auto Height() const noexcept;

		private:
			[[nodiscard]] static
			auto CheckFullTransparency(std::span<unsigned char const> data) -> bool;

			unsigned m_TexName;
			bool m_SemiTransparent;
			bool m_Transparent;
			std::filesystem::path m_Path;
			int m_Width;
			int m_Height;
	};


	inline auto Texture::operator<=>(Texture const& other) const
	{
		return m_Path <=> other.m_Path;
	}


	inline auto Texture::operator==(Texture const& other) const -> bool
	{
		return m_Path == other.m_Path;
	}


	constexpr auto Texture::Id() const
	{
		return m_TexName;
	}


	constexpr auto Texture::IsSemiTransparent() const
	{
		return m_SemiTransparent;
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
	auto operator<=>(Texture const& tex, std::filesystem::path const& path)
	{
		return tex.Path() <=> path;
	}


	// Provides ordering with paths.
	[[nodiscard]] inline
	auto operator<=>(std::filesystem::path const& path, Texture const& tex)
	{
		return path <=> tex.Path();
	}


	// Provides equality with paths.
	[[nodiscard]] inline
	auto operator==(Texture const& tex, std::filesystem::path const& path) -> bool
	{
		return tex.Path() == path;
	}


	// Provides equality with paths.
	[[nodiscard]] inline
	auto operator==(std::filesystem::path const& path, Texture const& tex) -> bool
	{
		return path == tex.Path();
	}
}
