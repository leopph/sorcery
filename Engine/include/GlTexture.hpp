#pragma once

#include "Image.hpp"
#include "LeopphApi.hpp"
#include "Types.hpp"


namespace leopph
{
	class GlTexture
	{
		public:
			explicit LEOPPHAPI GlTexture(Image const& img);

			[[nodiscard]] LEOPPHAPI u32 TextureName() const noexcept;
			[[nodiscard]] LEOPPHAPI u32 Width() const noexcept;
			[[nodiscard]] LEOPPHAPI u32 Height() const noexcept;

			GlTexture(GlTexture const& other) = delete;
			GlTexture& operator=(GlTexture const& other) = delete;

			GlTexture(GlTexture&& other) = delete;
			GlTexture& operator=(GlTexture&& other) = delete;

			~GlTexture() noexcept;

		private:
			u32 m_Texture;
			u32 m_Width;
			u32 m_Height;
	};
}
