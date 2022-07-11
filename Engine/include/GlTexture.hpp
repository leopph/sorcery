#pragma once

#include "Image.hpp"


namespace leopph
{
	enum class ColorSpace
	{
		linearRGB,
		sRGB
	};

	class GlTexture
	{
		public:
			explicit GlTexture(Image const& img, ColorSpace colorSpace = ColorSpace::linearRGB);

			GlTexture(GlTexture const& other) = delete;
			auto operator=(GlTexture const& other) -> GlTexture& = delete;

			GlTexture(GlTexture&& other) = delete;
			auto operator=(GlTexture&& other) -> GlTexture& = delete;

			~GlTexture() noexcept;

			// The name of the underlying OpenGL texture.
			[[nodiscard]]
			auto TextureName() const noexcept -> unsigned;

			[[nodiscard]]
			auto Width() const noexcept -> int;

			[[nodiscard]]
			auto Height() const noexcept -> int;

		private:
			unsigned m_Texture;
			int m_Width;
			int m_Height;
	};
}
