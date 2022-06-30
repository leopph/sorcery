#pragma once

#include "GlBufferObject.hpp"
#include "GlVertexArrayObject.hpp"

#include <array>


namespace leopph::internal
{
	// A quad that fills the entire screen.
	class GlScreenQuad
	{
		public:
			GlScreenQuad();

			GlScreenQuad(GlScreenQuad const& other) = delete;
			auto operator=(GlScreenQuad const& other) -> GlScreenQuad& = delete;

			GlScreenQuad(GlScreenQuad&& other) noexcept = delete;
			auto operator=(GlScreenQuad&& other) noexcept -> GlScreenQuad& = delete;

			~GlScreenQuad() noexcept = default;

			auto Draw() const -> void;

		private:
			GlVertexArrayObject m_VertexArray;
			GlBufferObject m_VertexBuffer;

			static constexpr std::array<float, 20> QUAD_VERTICES
			{
				-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
	};
}
