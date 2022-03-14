#pragma once

#include "../events/PointShadowEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../rendering/shaders/ShaderProgram.hpp"

#include <glad/gl.h>

#include <limits>
#include <string_view>


namespace leopph::internal
{
	class CubeShadowMap final : EventReceiver<PointShadowEvent>
	{
		public:
			CubeShadowMap();

			CubeShadowMap(const CubeShadowMap& other) = delete;
			auto operator=(const CubeShadowMap& other) -> CubeShadowMap& = delete;

			CubeShadowMap(CubeShadowMap&& other) = delete;
			auto operator=(CubeShadowMap&& other) -> CubeShadowMap& = delete;

			~CubeShadowMap() noexcept override;

			// Binds the passed face of the cubemap as render target.
			// Clears the face and depth buffer.
			auto BindForWritingAndClear(GLint face) const -> void;

			// Binds the cubemap to the passed texture unit, sets the passed uniform, and returns the next available texture unit.
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, std::string_view uniformName, GLuint texUnit) const -> GLuint;

		private:
			auto Init() -> void;
			auto Deinit() const -> void;
			auto OnEventReceived(EventParamType event) -> void override;

			GLuint m_Framebuffer;
			GLuint m_Cubemap;
			GLuint m_DepthBuffer;
			GLsizei m_Res;

			constexpr static GLfloat CLEAR_DEPTH{1};
			constexpr static GLfloat CLEAR_COLOR[4]
			{
				std::numeric_limits<GLfloat>::max(),
				std::numeric_limits<GLfloat>::max(),
				std::numeric_limits<GLfloat>::max(),
				std::numeric_limits<GLfloat>::max()
			};
	};
}
