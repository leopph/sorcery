#pragma once

#include "../events/PointShadowResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../rendering/shaders/ShaderProgram.hpp"

#include <glad/glad.h>


namespace leopph::internal
{
	class CubeShadowMap final : EventReceiver<PointShadowResolutionEvent>
	{
		public:
			CubeShadowMap();

			CubeShadowMap(const CubeShadowMap& other) = delete;
			CubeShadowMap(CubeShadowMap&& other) = delete;

			auto operator=(const CubeShadowMap& other) -> CubeShadowMap& = delete;
			auto operator=(CubeShadowMap&& other) -> CubeShadowMap& = delete;

			~CubeShadowMap() noexcept override;

			// Binds the cubemap as render target and sets its value to the default.
			auto BindForWritingAndClear() const -> void;

			// Binds the cubemap to the passed texture unit, sets the shader uniform, and returns the next available texture unit.
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, GLuint texUnit) const -> GLuint;

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
			constexpr static const char* SHADER_SHADOW_MAP_NAME{"u_ShadowMap"};
	};
}
