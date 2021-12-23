#pragma once

#include "../events/ScreenResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Vector.hpp"
#include "shaders/ShaderProgram.hpp"

#include <array>


namespace leopph::internal
{
	class GeometryBuffer final : public EventReceiver<ScreenResolutionEvent>
	{
		public:
			enum TextureType
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
			auto UnbindFromWriting() const -> void;

			// Returns the next available texture unit after binding
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, TextureType type, int texUnit) const -> int;
			// Returns the next available texture unit after binding
			[[nodiscard]] auto BindForReading(ShaderProgram& shader, int texUnit) const -> int;
			auto UnbindFromReading(TextureType type) const -> void;
			auto UnbindFromReading() const -> void;

			auto CopyDepthData(unsigned bufferName) const -> void;

		private:
			std::array<unsigned, 6> m_Textures;
			mutable std::array<int, std::tuple_size_v<decltype(m_Textures)>> m_BindIndices;
			unsigned m_DepthBuffer;
			unsigned m_FrameBuffer;
			Vector2 m_Resolution;

			auto SetUpBuffers(const Vector2& res) -> void;
			auto OnEventReceived(EventParamType event) -> void override;

			static constexpr int BIND_FILL_VALUE{-1};
	};
}
