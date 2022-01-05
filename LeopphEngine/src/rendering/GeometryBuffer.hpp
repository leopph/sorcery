#pragma once

#include "../events/ScreenResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Vector.hpp"
#include "shaders/ShaderProgram.hpp"

#include <glad/glad.h>

#include <array>


namespace leopph::internal
{
	class GeometryBuffer final : public EventReceiver<ScreenResolutionEvent>
	{
		public:
			enum class Texture
			{
				Position, NormalAndShine, Ambient, Diffuse, Specular
			};


			GeometryBuffer();

			GeometryBuffer(const GeometryBuffer&) = delete;
			GeometryBuffer(GeometryBuffer&&) = delete;

			auto operator=(const GeometryBuffer&) -> GeometryBuffer& = delete;
			auto operator=(GeometryBuffer&&) -> GeometryBuffer& = delete;

			~GeometryBuffer() override;

			// Binds the gbuffer as MRT render target and sets its values to the defaults.
			auto BindForWritingAndClear() const -> void;

			// Bind all textures and returns the next available texture unit after binding.
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, GLuint texUnit) const -> GLuint;

			auto CopyStencilData(GLuint bufferName) const -> void;

		private:
			using ResType = Vector<GLsizei, 2>;

			auto SetUpBuffers() -> void;
			auto OnEventReceived(EventParamType event) -> void override;

			std::array<GLuint, 5> m_Textures;
			GLuint m_DepthStencilBuffer;
			GLuint m_FrameBuffer;
			ResType m_Res;
			
			static constexpr GLfloat CLEAR_COLOR[]{0, 0, 0, 1};
			static constexpr GLdouble CLEAR_DEPTH{1};
			static constexpr GLuint CLEAR_STENCIL{1};
			static constexpr std::array SHADER_UNIFORM_NAMES
			{
				"u_PosTex", "u_NormShineTex", "u_AmbTex", "u_DiffTex", "u_SpecTex"
			};
	};
}
