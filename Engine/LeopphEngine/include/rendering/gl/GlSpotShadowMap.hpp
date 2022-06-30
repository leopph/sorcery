#pragma once

#include "EventReceiver.hpp"
#include "GlFramebufferObject.hpp"
#include "GlTextureObject.hpp"
#include "events/SpotShadowEvent.hpp"
#include "rendering/shaders/ShaderProgram.hpp"

#include <string_view>


namespace leopph::internal
{
	class GlSpotShadowMap final : EventReceiver<SpotShadowEvent>
	{
		public:
			GlSpotShadowMap();

			GlSpotShadowMap(GlSpotShadowMap const&) = delete;
			auto operator=(GlSpotShadowMap const&) -> GlSpotShadowMap& = delete;

			GlSpotShadowMap(GlSpotShadowMap&& other) = delete;
			auto operator=(GlSpotShadowMap&& other) -> GlSpotShadowMap& = delete;

			~GlSpotShadowMap() noexcept override = default;

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

			GlFramebufferObject m_Framebuffer;
			GlTextureObject<GlTextureType::T2D> m_ShadowMap;
			GLsizei m_Res;
	};
}
