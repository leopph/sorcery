#pragma once

#include <glad/gl.h>

#include <type_traits>


namespace leopph::internal
{
	class GlVertexArray
	{
		public:
			GlVertexArray();

			GlVertexArray(GlVertexArray const& other) = delete;
			auto operator=(GlVertexArray const& other) -> GlVertexArray& = delete;

			// Sets other to 0.
			GlVertexArray(GlVertexArray&& other) noexcept;

			// Sets other to 0.
			auto operator=(GlVertexArray&& other) noexcept -> GlVertexArray&;

			~GlVertexArray() noexcept;

			// Decays to the name.
			[[nodiscard]]
			operator GLuint() const noexcept;

			[[nodiscard]]
			auto Name() const noexcept -> GLuint;

		private:
			GLuint m_Name;
	};


	static_assert(std::is_default_constructible_v<GlVertexArray>);
	static_assert(!std::is_copy_constructible_v<GlVertexArray>);
	static_assert(!std::is_copy_assignable_v<GlVertexArray>);
	static_assert(std::is_move_constructible_v<GlVertexArray>);
	static_assert(std::is_move_assignable_v<GlVertexArray>);
}
