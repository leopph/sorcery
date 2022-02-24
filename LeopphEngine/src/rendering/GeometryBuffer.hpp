#pragma once

#include "../events/WindowEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Vector.hpp"
#include "shaders/ShaderProgram.hpp"

#include <glad/glad.h>

#include <array>


namespace leopph::internal
{
	class GeometryBuffer final : public EventReceiver<WindowEvent>
	{
		public:
			GeometryBuffer();

			GeometryBuffer(const GeometryBuffer&) = delete;
			GeometryBuffer(GeometryBuffer&&) = delete;

			auto operator=(const GeometryBuffer&) -> GeometryBuffer& = delete;
			auto operator=(GeometryBuffer&&) -> GeometryBuffer& = delete;

			~GeometryBuffer() noexcept override;

			// Binds the gbuffer as MRT render target and sets its values to the defaults.
			auto BindForWritingAndClear() const -> void;

			// Bind all textures and returns the next available texture unit after binding.
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, GLuint texUnit) const -> GLuint;

			auto CopyStencilData(GLuint bufferName) const -> void;

		private:
			using ResType = Vector<GLsizei, 2>;

			auto InitBuffers() noexcept -> void;
			auto DeinitBuffers() const noexcept -> void;
			auto OnEventReceived(EventParamType event) -> void override;

			std::array<GLuint, 2> m_Textures;
			GLuint m_FrameBuffer;
			ResType m_Res;
			
			static constexpr GLfloat CLEAR_COLOR[]{0, 0, 0, 1};
			static constexpr GLfloat CLEAR_DEPTH{1};
			static constexpr GLuint CLEAR_STENCIL{1};
			
			static constexpr unsigned NORM_COLOR_GLOSS_TEX{0};
			static constexpr unsigned DEPTH_STENCIL_TEX{1};

			static constexpr std::array SHADER_UNIFORM_NAMES
			{
				"u_NormColorGlossTex", "u_DepthTex"
			};
	};
}
