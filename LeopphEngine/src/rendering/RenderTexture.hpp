#pragma once

#include "../events/ScreenResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Vector.hpp"
#include "shaders/ShaderProgram.hpp"


namespace leopph::impl
{
	class RenderTexture final : public EventReceiver<ScreenResolutionEvent>
	{
		public:
			RenderTexture();

			RenderTexture(const RenderTexture& other) = delete;
			RenderTexture(RenderTexture&& other) = delete;

			~RenderTexture() override;

			RenderTexture& operator=(const RenderTexture& other) = delete;
			RenderTexture& operator=(RenderTexture&& other) = delete;

			void DrawToTexture() const;
			void DrawToWindow() const;

			void BindAsRenderTarget() const;
			void UnbindAsRenderTarget() const;

			[[nodiscard]]
			unsigned FramebufferName() const;

			void Clear() const;


		private:
			void InitTextures(unsigned width, unsigned height);
			void DeinitTextures() const;

			void OnEventReceived(EventParamType event) override;

			unsigned m_FramebufferName;
			unsigned m_ColorTextureName;
			unsigned m_DepthBufferName;
			unsigned m_VertexArrayName;
			unsigned m_VertexBufferName;
			Vector2 m_Resolution;

			static constexpr std::array<float, 20> QUAD_VERTICES
			{
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
	};
}