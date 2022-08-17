#pragma once

#include "Image.hpp"
#include "Types.hpp"


namespace leopph
{
	class Texture2D
	{
		public:
			[[nodiscard]] u64 get_handle() const;
			[[nodiscard]] u32 get_width() const;
			[[nodiscard]] u32 get_height() const;

			explicit Texture2D(Image const& img);
			Texture2D(Texture2D const& other) = delete;
			Texture2D(Texture2D&& other) = delete;

			~Texture2D();

			Texture2D& operator=(Texture2D const& other) = delete;
			Texture2D& operator=(Texture2D&& other) = delete;

		private:
			u64 mHandle{};
			u32 mTexture{};
			u32 mWidth;
			u32 mHeight;
	};
}
