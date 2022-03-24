#pragma once

#include "OpenGl.hpp"

#include <type_traits>


namespace leopph::internal
{
	class GlFramebuffer
	{
		public:
			GlFramebuffer();

			GlFramebuffer(GlFramebuffer const& other) = delete;
			auto operator=(GlFramebuffer const& other) -> GlFramebuffer& = delete;

			// Sets other to 0.
			GlFramebuffer(GlFramebuffer&& other) noexcept;
			// Sets other to 0.
			auto operator=(GlFramebuffer&& other) noexcept -> GlFramebuffer&;

			~GlFramebuffer() noexcept;

			// Decays to the name.
			[[nodiscard]]
			operator GLuint() const noexcept;

			[[nodiscard]]
			auto Name() const noexcept -> GLuint;

		private:
			GLuint m_Name;
	};


	static_assert(std::is_default_constructible_v<GlFramebuffer>);
	static_assert(!std::is_copy_constructible_v<GlFramebuffer>);
	static_assert(!std::is_copy_assignable_v<GlFramebuffer>);
	static_assert(std::is_move_constructible_v<GlFramebuffer>);
	static_assert(std::is_move_assignable_v<GlFramebuffer>);
}
