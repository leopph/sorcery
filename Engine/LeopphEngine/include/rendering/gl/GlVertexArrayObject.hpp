#pragma once

#include "GlCore.hpp"

#include <type_traits>


namespace leopph::internal
{
	class GlVertexArrayObject
	{
		public:
			GlVertexArrayObject();

			GlVertexArrayObject(GlVertexArrayObject const& other) = delete;
			auto operator=(GlVertexArrayObject const& other) -> GlVertexArrayObject& = delete;

			// Sets other to 0.
			GlVertexArrayObject(GlVertexArrayObject&& other) noexcept;

			// Sets other to 0.
			auto operator=(GlVertexArrayObject&& other) noexcept -> GlVertexArrayObject&;

			~GlVertexArrayObject() noexcept;

			// Decays to the name.
			[[nodiscard]]
			operator GLuint() const noexcept;

			[[nodiscard]]
			auto Name() const noexcept -> GLuint;

		private:
			GLuint m_Name;
	};


	static_assert(std::is_default_constructible_v<GlVertexArrayObject>);
	static_assert(!std::is_copy_constructible_v<GlVertexArrayObject>);
	static_assert(!std::is_copy_assignable_v<GlVertexArrayObject>);
	static_assert(std::is_move_constructible_v<GlVertexArrayObject>);
	static_assert(std::is_move_assignable_v<GlVertexArrayObject>);
}
