#pragma once

#include "EventReceiver.hpp"
#include "../event/SpotShadowEvent.hpp"
#include "opengl/GlFramebuffer.hpp"
#include "opengl/GlTexture.hpp"
#include "shaders/ShaderProgram.hpp"

#include <string_view>


namespace leopph::internal
{
	class SpotShadowMap final : EventReceiver<SpotShadowEvent>
	{
		public:
			SpotShadowMap();

			SpotShadowMap(SpotShadowMap const&) = delete;
			auto operator=(SpotShadowMap const&) -> SpotShadowMap& = delete;

			SpotShadowMap(SpotShadowMap&& other) = delete;
			auto operator=(SpotShadowMap&& other) -> SpotShadowMap& = delete;

			~SpotShadowMap() noexcept override = default;

			// Clears the shadow map.
			auto Clear() const noexcept -> void;

			// Binds the shadow map for writing.
			auto BindForWriting() const noexcept -> void;

			// Binds the shadow map and render target and sets its value to the default.
			auto BindForWritingAndClear() const -> void;

			// Bind the shadow map to the passed texture unit, sets the passed uniform, and returns the next available texture unit.
			[[nodiscard]]
			auto BindForReading(ShaderProgram& shader, std::string_view uniformName, GLuint texUnit) const -> GLuint;

		private:
			// Configures the shadow map to the current resolution.
			auto ConfigureShadowMap() -> void;

			// Updates the shadow map to the new resolution.
			auto OnEventReceived(EventParamType event) -> void override;

			GlFramebuffer m_Framebuffer;
			GlTexture<GlTextureType::T2D> m_ShadowMap;
			GLsizei m_Res;
	};
}
