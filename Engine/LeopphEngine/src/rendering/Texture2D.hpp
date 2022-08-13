#pragma once

#include "Image.hpp"
#include "Types.hpp"


namespace leopph
{
	class Texture2D
	{
		public:
			explicit Texture2D(Image const& img);

			Texture2D(Texture2D const& other) = delete;
			Texture2D& operator=(Texture2D const& other) = delete;

			Texture2D(Texture2D&& other) = delete;
			Texture2D& operator=(Texture2D&& other) = delete;

			~Texture2D();

			[[nodiscard]] u32 get_handle() const;
			[[nodiscard]] u32 get_width() const;
			[[nodiscard]] u32 get_height() const;

		private:
			u32 mTexture{};
			u32 mHandle{};
			u32 mWidth;
			u32 mHeight;
	};
}
