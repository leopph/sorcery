#pragma once

#include "GlCore.hpp"

#include <type_traits>


namespace leopph::internal
{
	class GlRenderbufferObject
	{
		public:
			GlRenderbufferObject();

			GlRenderbufferObject(GlRenderbufferObject const& other) = delete;
			auto operator=(GlRenderbufferObject const& other) -> GlRenderbufferObject& = delete;

			// Sets other to 0.
			GlRenderbufferObject(GlRenderbufferObject&& other) noexcept;
			// Sets other to 0.
			auto operator=(GlRenderbufferObject&& other) noexcept -> GlRenderbufferObject&;

			~GlRenderbufferObject() noexcept;

			// Decays to the name.
			[[nodiscard]]
			operator GLuint() const noexcept;

			[[nodiscard]]
			auto Name() const noexcept -> GLuint;

		private:
			GLuint m_Name;
	};


	static_assert(std::is_default_constructible_v<GlRenderbufferObject>);
	static_assert(!std::is_copy_constructible_v<GlRenderbufferObject>);
	static_assert(!std::is_copy_assignable_v<GlRenderbufferObject>);
	static_assert(std::is_move_constructible_v<GlRenderbufferObject>);
	static_assert(std::is_move_assignable_v<GlRenderbufferObject>);
}
