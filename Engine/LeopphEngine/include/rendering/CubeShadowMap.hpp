#pragma once

#include "EventReceiver.hpp"
#include "events/PointShadowEvent.hpp"
#include "opengl/GlFramebuffer.hpp"
#include "opengl/GlRenderbuffer.hpp"
#include "opengl/GlTexture.hpp"
#include "rendering/shaders/ShaderProgram.hpp"

#include <string_view>


namespace leopph::internal
{
	class CubeShadowMap final : EventReceiver<PointShadowEvent>
	{
		public:
			CubeShadowMap();

			CubeShadowMap(CubeShadowMap const& other) = delete;
			auto operator=(CubeShadowMap const& other) -> CubeShadowMap& = delete;

			CubeShadowMap(CubeShadowMap&& other) = delete;
			auto operator=(CubeShadowMap&& other) -> CubeShadowMap& = delete;

			~CubeShadowMap() noexcept override = default;

			// Clear the depth buffer and the currently bound face of the cube map.
			auto Clear() const noexcept -> void;

			// Binds the passed face of the cube map for writing.
			auto BindForWriting(GLint face) const noexcept -> void;

			// Binds the passed face of the cubemap as render target.
			// Clears the face and depth buffer.
			auto BindForWritingAndClear(GLint face) const -> void;

			// Binds the cubemap to the passed texture unit, sets the passed uniform, and returns the next available texture unit.
			[[nodiscard]]
			auto BindForReading(ShaderProgram& shader, std::string_view uniformName, GLuint texUnit) const -> GLuint;

		private:
			// Reconfigures the cube map to the current resolution.
			auto ConfigCubeMap() -> void;

			// Configures the cube map to the new resolution.
			auto OnEventReceived(EventParamType event) -> void override;

			GlFramebuffer m_Framebuffer;
			GlTexture<GlTextureType::CubeMap> m_Cubemap;
			GlRenderbuffer m_DepthBuffer;
			GLsizei m_Res;
	};
}
