#pragma once

#include "../events/DisplayResolutionChangedEvent.hpp"
#include "../events/EventReceiver.hpp"
#include "../math/Vector.hpp"
#include "shaders/DefLightShader.hpp"

#include <array>



namespace leopph::impl
{
	class GeometryBuffer final : public EventReceiver<DisplayResolutionChangedEvent>
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
			[[nodiscard]] int BindForReading(const DefLightShader& shader, TextureType type, int texUnit) const;
			// Returns the next available texture unit after binding
			[[nodiscard]] int BindForReading(const DefLightShader& shader, int texUnit) const;
			void UnbindFromReading(TextureType type) const;
			void UnbindFromReading() const;

			void CopyDepthData(unsigned bufferName, const Vector2& resolution) const;


		private:
			std::array<unsigned, 6> m_Textures;
			mutable std::array<int, std::tuple_size_v<decltype(m_Textures)>> m_BindIndices;
			unsigned m_DepthBuffer;
			unsigned m_FrameBuffer;
			Vector2 m_Resolution;

			void SetUpBuffers(const Vector2& res);
			void OnEventReceived(EventParamType event) override;

			static const int s_BindFillValue;
	};
}
