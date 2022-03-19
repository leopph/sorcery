#pragma once

#include <glad/gl.h>

#include <type_traits>


namespace leopph::internal
{
	class GlRenderbuffer
	{
		public:
			GlRenderbuffer();

			GlRenderbuffer(GlRenderbuffer const& other) = delete;
			auto operator=(GlRenderbuffer const& other) -> GlRenderbuffer& = delete;

			// Sets other to 0.
			GlRenderbuffer(GlRenderbuffer&& other) noexcept;
			// Sets other to 0.
			auto operator=(GlRenderbuffer&& other) noexcept -> GlRenderbuffer&;

			~GlRenderbuffer() noexcept;

			// Decays to the name.
			[[nodiscard]]
			operator GLuint() const noexcept;

			[[nodiscard]]
			auto Name() const noexcept -> GLuint;

		private:
			GLuint m_Name;
	};


	static_assert(std::is_default_constructible_v<GlRenderbuffer>);
	static_assert(!std::is_copy_constructible_v<GlRenderbuffer>);
	static_assert(!std::is_copy_assignable_v<GlRenderbuffer>);
	static_assert(std::is_move_constructible_v<GlRenderbuffer>);
	static_assert(std::is_move_assignable_v<GlRenderbuffer>);
}
