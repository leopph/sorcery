#pragma once

#include "opengl/GlBuffer.hpp"
#include "opengl/GlVertexArray.hpp"

#include <array>


namespace leopph::internal
{
	// A quad that fills the entire screen.
	class ScreenQuad
	{
		public:
			ScreenQuad();

			ScreenQuad(ScreenQuad const& other) = delete;
			auto operator=(ScreenQuad const& other) -> ScreenQuad& = delete;

			ScreenQuad(ScreenQuad&& other) noexcept = delete;
			auto operator=(ScreenQuad&& other) noexcept -> ScreenQuad& = delete;

			~ScreenQuad() noexcept = default;

			auto Draw() const -> void;

		private:
			GlVertexArray m_VertexArray;
			GlBuffer m_VertexBuffer;

			static constexpr std::array<float, 20> QUAD_VERTICES
			{
				-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
	};
}
