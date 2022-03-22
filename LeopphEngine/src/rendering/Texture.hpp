#pragma once

#include "Image.hpp"

#include <span>


namespace leopph
{
	class Texture
	{
		public:
			explicit Texture(Image const& img);

			Texture(Texture const& other) = delete;
			auto operator=(Texture const& other) -> Texture& = delete;

			Texture(Texture&& other) = delete;
			auto operator=(Texture&& other) -> Texture& = delete;

			~Texture() noexcept;

			// The name of the underlying OpenGL texture.
			[[nodiscard]]
			auto TextureName() const -> unsigned;

			[[nodiscard]]
			auto Width() const noexcept -> int;

			[[nodiscard]]
			auto Height() const noexcept -> int;

			// A Texture is semi-transparent if it contains pixels
			// with an alpha value less than 1.
			[[nodiscard]]
			auto IsSemiTransparent() const -> bool;

			// A Texture is transparent if all of its pixels have
			// an alpha values less than 1.
			[[nodiscard]]
			auto IsTransparent() const -> bool;

		private:
			[[nodiscard]] static
			auto CheckSemiTransparency(std::span<unsigned char const> data) -> bool;

			[[nodiscard]] static
			auto CheckFullTransparency(std::span<unsigned char const> data) -> bool;

			unsigned m_Texture;
			bool m_SemiTransparent;
			bool m_Transparent;
			int m_Width;
			int m_Height;
	};
}
