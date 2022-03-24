#pragma once

#include <GL/gl3w.h>

#include <type_traits>


namespace leopph::internal
{
	class GlBuffer
	{
		public:
			GlBuffer();

			GlBuffer(GlBuffer const& other) = delete;
			auto operator=(GlBuffer const& other) -> GlBuffer& = delete;

			// Sets other to 0.
			GlBuffer(GlBuffer&& other) noexcept;

			// Sets other to 0.
			auto operator=(GlBuffer&& other) noexcept -> GlBuffer&;

			~GlBuffer() noexcept;

			// Decays to the name.
			[[nodiscard]]
			operator GLuint() const noexcept;

			[[nodiscard]]
			auto Name() const noexcept -> GLuint;

		private:
			GLuint m_Name;
	};


	static_assert(std::is_default_constructible_v<GlBuffer>);
	static_assert(!std::is_copy_constructible_v<GlBuffer>);
	static_assert(!std::is_copy_assignable_v<GlBuffer>);
	static_assert(std::is_move_constructible_v<GlBuffer>);
	static_assert(std::is_move_assignable_v<GlBuffer>);
}
