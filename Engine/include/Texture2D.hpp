#pragma once

#include "Image.hpp"
#include "LeopphApi.hpp"
#include "Types.hpp"


namespace leopph
{
	class Texture2D
	{
		public:
			explicit LEOPPHAPI Texture2D(Image const& img);

			Texture2D(Texture2D const& other) = delete;
			Texture2D& operator=(Texture2D const& other) = delete;

			Texture2D(Texture2D&& other) = delete;
			Texture2D& operator=(Texture2D&& other) = delete;

			~Texture2D();

			[[nodiscard]] LEOPPHAPI u32 internal_handle() const;
			[[nodiscard]] LEOPPHAPI u32 width() const;
			[[nodiscard]] LEOPPHAPI u32 height() const;

		private:
			u32 mTexture{};
			u32 mHandle{};
			u32 mWidth;
			u32 mHeight;
	};
}
