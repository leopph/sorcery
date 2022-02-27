#pragma once

#include "../events/PointShadowResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../rendering/shaders/ShaderProgram.hpp"

#include <glad/gl.h>

#include <string_view>


namespace leopph::internal
{
	class CubeShadowMap final : EventReceiver<PointShadowResolutionEvent>
	{
		public:
			CubeShadowMap();

			CubeShadowMap(const CubeShadowMap& other) = delete;
			auto operator=(const CubeShadowMap& other) -> CubeShadowMap& = delete;

			CubeShadowMap(CubeShadowMap&& other) = delete;
			auto operator=(CubeShadowMap&& other) -> CubeShadowMap& = delete;

			~CubeShadowMap() noexcept override;

			// Binds the cubemap as render target and sets its value to the default.
			auto BindForWritingAndClear() const -> void;

			// Binds the cubemap to the passed texture unit, sets the passed uniform, and returns the next available texture unit.
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, std::string_view uniformName, GLuint texUnit) const -> GLuint;

		private:
			// Initializes the cubemap to the current resolution.
			auto InitCubemap() -> void;
			// Destroys the cubemap.
			auto DeinitCubemap() const -> void;
			auto OnEventReceived(EventParamType event) -> void override;

			GLuint m_Framebuffer;
			GLuint m_Cubemap;
			GLsizei m_Res;

			constexpr static GLfloat CLEAR_DEPTH{1};
	};
}
