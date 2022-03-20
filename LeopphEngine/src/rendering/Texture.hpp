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
			[[nodiscard]]
			auto operator<=>(Texture const& other) const -> std::strong_ordering;

			// Equality based on path.
			[[nodiscard]]
			auto operator==(Texture const& other) const -> bool;

			// A Texture is semi-transparent if it contains pixels
			// with an alpha value less than 1.
			[[nodiscard]]
			auto IsSemiTransparent() const -> bool;

			// A Texture is transparent if all of its pixels have
			// an alpha values less than 1.
			[[nodiscard]]
			auto IsTransparent() const -> bool;

			// The name of the underlying OpenGL texture.
			[[nodiscard]]
			auto TextureName() const -> unsigned;

			// The source file path.
			[[nodiscard]]
			auto Path() const -> std::filesystem::path const&;

			[[nodiscard]]
			auto Width() const noexcept -> int;

			[[nodiscard]]
			auto Height() const noexcept -> int;

		private:
			[[nodiscard]] static
			auto CheckSemiTransparency(std::span<unsigned char const> data) -> bool;

			[[nodiscard]] static
			auto CheckFullTransparency(std::span<unsigned char const> data) -> bool;

			unsigned m_Texture;
			std::filesystem::path m_Path;
			bool m_SemiTransparent;
			bool m_Transparent;
			int m_Width;
			int m_Height;
	};


	// Provides ordering with paths.
	[[nodiscard]]
	auto operator<=>(Texture const& tex, std::filesystem::path const& path) -> std::strong_ordering;

	// Provides ordering with paths.
	[[nodiscard]]
	auto operator<=>(std::filesystem::path const& path, Texture const& tex) -> std::strong_ordering;

	// Provides equality with paths.
	[[nodiscard]]
	auto operator==(Texture const& tex, std::filesystem::path const& path) -> bool
	;

	// Provides equality with paths.
	[[nodiscard]]
	auto operator==(std::filesystem::path const& path, Texture const& tex) -> bool
	;
}
