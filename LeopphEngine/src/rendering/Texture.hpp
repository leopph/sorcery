#pragma once

#include "Image.hpp"


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
