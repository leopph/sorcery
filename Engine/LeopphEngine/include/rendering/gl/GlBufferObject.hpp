#pragma once

#include "GlCore.hpp"

#include <type_traits>


namespace leopph::internal
{
	class GlBufferObject
	{
		public:
			GlBufferObject();

			GlBufferObject(GlBufferObject const& other) = delete;
			GlBufferObject& operator=(GlBufferObject const& other) = delete;

			// Sets other to 0.
			GlBufferObject(GlBufferObject&& other) noexcept;

			// Sets other to 0.
			GlBufferObject& operator=(GlBufferObject&& other) noexcept;

			~GlBufferObject() noexcept;

			// Decays to the name.
			[[nodiscard]]
			operator GLuint() const noexcept;

			[[nodiscard]]
			GLuint Name() const noexcept;

		private:
			GLuint m_Name;
	};


	static_assert(std::is_default_constructible_v<GlBufferObject>);
	static_assert(!std::is_copy_constructible_v<GlBufferObject>);
	static_assert(!std::is_copy_assignable_v<GlBufferObject>);
	static_assert(std::is_move_constructible_v<GlBufferObject>);
	static_assert(std::is_move_assignable_v<GlBufferObject>);
}
