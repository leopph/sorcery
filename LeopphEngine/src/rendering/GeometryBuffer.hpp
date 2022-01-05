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

			auto Clear() const -> void;

			auto BindForWriting() const -> void;
			static auto UnbindFromWriting() -> void;

			// Binds the passed texture and returns the next available texture unit after binding
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, Texture type, int texUnit) const -> int;
			// Bind all textures and returns the next available texture unit after binding
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, int texUnit) const -> int;

			auto UnbindFromReading(Texture type) const -> void;
			auto UnbindFromReading() const -> void;
			
			auto CopyStencilData(GLuint bufferName) const -> void;

		private:
			using ResType = Vector<GLsizei, 2>;

			auto SetUpBuffers() -> void;
			auto OnEventReceived(EventParamType event) -> void override;

			std::array<GLuint, 5> m_Textures;
			mutable std::array<int, std::tuple_size_v<decltype(m_Textures)>> m_BindIndices;
			GLuint m_DepthStencilBuffer;
			GLuint m_FrameBuffer;
			ResType m_Res;

			static constexpr int BIND_FILL_VALUE{-1};
			static constexpr GLfloat CLEAR_COLOR[]{0.f, 0.f, 0.f, 1.f};
			static constexpr GLfloat CLEAR_DEPTH{1.f};
			static constexpr GLuint CLEAR_STENCIL{1};
	};
}
