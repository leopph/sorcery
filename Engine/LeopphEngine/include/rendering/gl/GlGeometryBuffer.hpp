#pragma once

#include "EventReceiver.hpp"
#include "GlCore.hpp"
#include "GlFramebufferObject.hpp"
#include "GlTextureObject.hpp"
#include "WindowEvent.hpp"
#include "rendering/shaders/ShaderProgram.hpp"


namespace leopph::internal
{
	class GlGeometryBuffer final : public EventReceiver<WindowEvent>
	{
		public:
			GlGeometryBuffer();

			GlGeometryBuffer(GlGeometryBuffer const&) = delete;
			GlGeometryBuffer(GlGeometryBuffer&&) = delete;

			auto operator=(GlGeometryBuffer const&) -> GlGeometryBuffer& = delete;
			auto operator=(GlGeometryBuffer&&) -> GlGeometryBuffer& = delete;

			~GlGeometryBuffer() noexcept override = default;

			[[nodiscard]]
			auto Framebuffer() const noexcept -> GlFramebufferObject const&;

			[[nodiscard]]
			auto NormalColorGlossTexture() const noexcept -> GlTextureObject<GlTextureType::T2D> const&;

			[[nodiscard]]
			auto DepthStencilTexture() const noexcept -> GlTextureObject<GlTextureType::T2D> const&;

			[[nodiscard]]
			auto Width() const noexcept -> int;

			[[nodiscard]]
			auto Height() const noexcept -> int;

			// Clears all buffers.
			auto Clear() const -> void;

			// Binds the buffers for writing.
			auto BindForWriting() const -> void;

			// Bind all textures and returns the next available texture unit after binding.
			[[nodiscard]]
			auto BindForReading(ShaderProgram& shader, GLuint texUnit) const -> GLuint;

		private:
			// Configures the buffers for the current resolution.
			auto ConfigBuffers() noexcept -> void;

			// Reconfigures the buffers to the new resolution.
			auto OnEventReceived(EventParamType event) -> void override;

			GlFramebufferObject m_Framebuffer;
			GlTextureObject<GlTextureType::T2D> m_NormClrGlossTexture;
			GlTextureObject<GlTextureType::T2D> m_DepthStencilTexture;
			int m_Width;
			int m_Height;
	};
}
