#pragma once

#include "../events/ScreenResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Vector.hpp"
#include "shaders/ShaderProgram.hpp"

#include <glad/glad.h>

#include <array>


namespace leopph::internal
{
	class RenderTexture final : public EventReceiver<ScreenResolutionEvent>
	{
		public:
			RenderTexture();

			RenderTexture(const RenderTexture& other) = delete;
			auto operator=(const RenderTexture& other) -> RenderTexture& = delete;

			RenderTexture(RenderTexture&& other) = delete;
			auto operator=(RenderTexture&& other) -> RenderTexture& = delete;

			~RenderTexture() override;

			auto DrawToTexture() const -> void;
			auto DrawToWindow() const -> void;

			auto BindAsRenderTarget() const -> void;
			static auto UnbindAsRenderTarget() -> void;

			auto Clear() const -> void;

			[[nodiscard]] auto FramebufferName() const -> unsigned;

		private:
			auto OnEventReceived(EventParamType event) -> void override;
			auto InitBuffers(GLsizei width, GLsizei height) -> void;
			auto DeinitBuffers() const -> void;

			GLuint m_Framebuffer;
			GLuint m_ColorBuffer;
			GLuint m_DepthStencilBuffer;
			GLuint m_VertexArray;
			GLuint m_VertexBuffer;
			Vector2 m_Resolution;

			static constexpr GLfloat CLEAR_DEPTH{1.f};
			static constexpr GLuint CLEAR_STENCIL{1};
			static constexpr std::array<float, 20> QUAD_VERTICES
			{
				-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
	};
}
