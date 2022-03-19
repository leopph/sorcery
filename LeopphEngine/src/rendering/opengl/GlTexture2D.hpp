#pragma once

#include <glad/gl.h>

#include <type_traits>


namespace leopph::internal
{
	// Handle to an OpenGL 2D texture object.
	class GlTexture2D
	{
		public:
			GlTexture2D();

			GlTexture2D(GlTexture2D const& other) = delete;
			auto operator=(GlTexture2D const& other) -> GlTexture2D& = delete;

			// Sets the other to 0.
			GlTexture2D(GlTexture2D&& other) noexcept;
			// Sets the other to 0.
			auto operator=(GlTexture2D&& other) noexcept -> GlTexture2D&;

			~GlTexture2D() noexcept;

			// Decays to the name.
			[[nodiscard]]
			operator GLuint() const noexcept;

			[[nodiscard]]
			auto Name() const noexcept -> GLuint;

		private:
			GLuint m_Name;
	};


	static_assert(std::is_default_constructible_v<GlTexture2D>);
	static_assert(!std::is_copy_constructible_v<GlTexture2D>);
	static_assert(!std::is_copy_assignable_v<GlTexture2D>);
	static_assert(std::is_move_constructible_v<GlTexture2D>);
	static_assert(std::is_move_assignable_v<GlTexture2D>);
}
