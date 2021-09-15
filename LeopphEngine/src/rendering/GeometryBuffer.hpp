#pragma once

#include "../events/DisplayResolutionChangedEvent.hpp"
#include "../events/EventReceiver.hpp"
#include "../math/Vector.hpp"
#include "shaders/DeferredLightShader.hpp"

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
		void Bind() const;
		void Unbind() const;

		// Returns the next available texture unit after binding
		[[nodiscard]] int BindTextureForReading(const DeferredLightShader& shader, TextureType type, int texUnit) const;

		void CopyDepthData(unsigned bufferName, const Vector2& resolution) const;


	private:
		std::array<unsigned, 6> m_Textures;
		unsigned m_DepthBuffer;
		unsigned m_FrameBuffer;

		void SetUpBuffers(const Vector2& res);
		void OnEventReceived(EventParamType event) override;
	};
}
