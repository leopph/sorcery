#pragma once

#include "GlCore.hpp"

#include <type_traits>


namespace leopph::internal
{
	class GlFramebufferObject
	{
		public:
			GlFramebufferObject();

			GlFramebufferObject(GlFramebufferObject const& other) = delete;
			auto operator=(GlFramebufferObject const& other) -> GlFramebufferObject& = delete;

			// Sets other to 0.
			GlFramebufferObject(GlFramebufferObject&& other) noexcept;
			// Sets other to 0.
			auto operator=(GlFramebufferObject&& other) noexcept -> GlFramebufferObject&;

			~GlFramebufferObject() noexcept;

			// Decays to the name.
			[[nodiscard]]
			operator GLuint() const noexcept;

			[[nodiscard]]
			auto Name() const noexcept -> GLuint;

		private:
			GLuint m_Name;
	};


	static_assert(std::is_default_constructible_v<GlFramebufferObject>);
	static_assert(!std::is_copy_constructible_v<GlFramebufferObject>);
	static_assert(!std::is_copy_assignable_v<GlFramebufferObject>);
	static_assert(std::is_move_constructible_v<GlFramebufferObject>);
	static_assert(std::is_move_assignable_v<GlFramebufferObject>);
}
