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

			GeometryBuffer& operator=(const GeometryBuffer&) = delete;
			GeometryBuffer& operator=(GeometryBuffer&&) = delete;

			~GeometryBuffer() override;

			void Clear() const;
			void BindForWriting() const;
			void UnbindFromWriting() const;

			// Returns the next available texture unit after binding
			[[nodiscard]] int BindForReading(ShaderProgram& shader, TextureType type, int texUnit) const;
			// Returns the next available texture unit after binding
			[[nodiscard]] int BindForReading(ShaderProgram& shader, int texUnit) const;
			void UnbindFromReading(TextureType type) const;
			void UnbindFromReading() const;

			void CopyDepthData(unsigned bufferName) const;


		private:
			std::array<unsigned, 6> m_Textures;
			mutable std::array<int, std::tuple_size_v<decltype(m_Textures)>> m_BindIndices;
			unsigned m_DepthBuffer;
			unsigned m_FrameBuffer;
			Vector2 m_Resolution;

			void SetUpBuffers(const Vector2& res);
			void OnEventReceived(EventParamType event) override;

			static constexpr int BIND_FILL_VALUE{-1};
	};
}
