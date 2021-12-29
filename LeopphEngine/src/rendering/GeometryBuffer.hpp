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
				Position, Normal, Ambient, Diffuse, Specular, Shine
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

			// Returns the next available texture unit after binding
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, Texture type, int texUnit) const -> int;
			// Returns the next available texture unit after binding
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, int texUnit) const -> int;

			auto UnbindFromReading(Texture type) const -> void;
			auto UnbindFromReading() const -> void;

			auto CopyDepthData(unsigned bufferName) const -> void;
			auto CopyStencilData(unsigned bufferName) const -> void;

		private:
			auto SetUpBuffers(const Vector2& res) -> void;
			auto OnEventReceived(EventParamType event) -> void override;

			std::array<GLuint, 6> m_Textures;
			mutable std::array<int, std::tuple_size_v<decltype(m_Textures)>> m_BindIndices;
			GLuint m_DepthBuffer;
			GLuint m_FrameBuffer;
			Vector2 m_Resolution;

			static constexpr int BIND_FILL_VALUE{-1};
			static constexpr GLfloat CLEAR_COLOR[]{0.f, 0.f, 0.f, 1.f};
			static constexpr GLfloat CLEAR_DEPTH{1.f};
			static constexpr GLuint CLEAR_STENCIL{1};
	};
}
