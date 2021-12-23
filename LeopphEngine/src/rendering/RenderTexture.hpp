#pragma once

#include "../events/ScreenResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Vector.hpp"
#include "shaders/ShaderProgram.hpp"


namespace leopph::internal
{
	class RenderTexture final : public EventReceiver<ScreenResolutionEvent>
	{
		public:
			RenderTexture();

			RenderTexture(const RenderTexture& other) = delete;
			RenderTexture(RenderTexture&& other) = delete;

			~RenderTexture() override;

			auto operator=(const RenderTexture& other) -> RenderTexture& = delete;
			auto operator=(RenderTexture&& other) -> RenderTexture& = delete;

			auto DrawToTexture() const -> void;
			auto DrawToWindow() const -> void;

			auto BindAsRenderTarget() const -> void;
			auto UnbindAsRenderTarget() const -> void;

			[[nodiscard]]
			auto FramebufferName() const -> unsigned;

			auto Clear() const -> void;

		private:
			auto InitTextures(unsigned width, unsigned height) -> void;
			auto DeinitTextures() const -> void;

			auto OnEventReceived(EventParamType event) -> void override;

			unsigned m_FramebufferName;
			unsigned m_ColorTextureName;
			unsigned m_DepthBufferName;
			unsigned m_VertexArrayName;
			unsigned m_VertexBufferName;
			Vector2 m_Resolution;

			static constexpr std::array<float, 20> QUAD_VERTICES
			{
				-1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
	};
}
