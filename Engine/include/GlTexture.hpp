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
			
			[[nodiscard]] auto LEOPPHAPI TextureName() const noexcept -> u32;
			[[nodiscard]] auto LEOPPHAPI Width() const noexcept -> u32;
			[[nodiscard]] auto LEOPPHAPI Height() const noexcept -> u32;

			GlTexture(GlTexture const& other) = delete;
			auto operator=(GlTexture const& other) -> GlTexture& = delete;

			GlTexture(GlTexture&& other) = delete;
			auto operator=(GlTexture&& other) -> GlTexture& = delete;

			~GlTexture() noexcept;

		private:
			u32 m_Texture;
			u32 m_Width;
			u32 m_Height;
	};
}
