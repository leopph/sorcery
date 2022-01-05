#pragma once

#include "../events/SpotShadowResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "shaders/ShaderProgram.hpp"

#include <glad/glad.h>

#include <string_view>


namespace leopph::internal
{
	class SpotShadowMap final : EventReceiver<SpotShadowResolutionEvent>
	{
		public:
			SpotShadowMap();

			SpotShadowMap(const SpotShadowMap&) = delete;
			auto operator=(const SpotShadowMap&) -> SpotShadowMap& = delete;

			SpotShadowMap(SpotShadowMap&& other) = delete;
			auto operator=(SpotShadowMap&& other) -> SpotShadowMap& = delete;

			~SpotShadowMap() noexcept override;

			// Binds the shadow map and render target and sets its value to the default.
			auto BindForWritingAndClear() const -> void;

			// Bind the shadow map to the passed texture unit, sets the passed uniform, and returns the next available texture unit.
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, std::string_view uniformName, GLuint texUnit) const -> GLuint;

		private:
			// Initializes the shadow map to the current resolution.
			auto InitShadowMap() -> void;
			// Destroys the shadow map.
			auto DeinitShadowMap() const -> void;
			auto OnEventReceived(EventParamType event) -> void override;

			unsigned m_Framebuffer;
			unsigned m_ShadowMap;
			GLsizei m_Res;

			constexpr static GLfloat CLEAR_DEPTH{1}; 
	};
}
