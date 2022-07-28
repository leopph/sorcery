#pragma once

#include "Image.hpp"
#include "LeopphApi.hpp"
#include "Types.hpp"


namespace leopph
{
	class Texture
	{
		public:
			explicit LEOPPHAPI Texture(Image const& img);

			[[nodiscard]] LEOPPHAPI u32 TextureName() const noexcept;
			[[nodiscard]] LEOPPHAPI u32 Width() const noexcept;
			[[nodiscard]] LEOPPHAPI u32 Height() const noexcept;

			Texture(Texture const& other) = delete;
			Texture& operator=(Texture const& other) = delete;

			Texture(Texture&& other) = delete;
			Texture& operator=(Texture&& other) = delete;

			~Texture() noexcept;

		private:
			u32 m_Texture;
			u32 m_Width;
			u32 m_Height;
	};
}
