#pragma once

#include "../math/Vector.hpp"

namespace leopph::impl
{
	class ShadowMap
	{
	public:
		explicit ShadowMap(const Vector2& resolution);
		~ShadowMap();

		ShadowMap(const ShadowMap&) = delete;
		ShadowMap(ShadowMap&& other) noexcept;
		void operator=(const ShadowMap&) = delete;
		ShadowMap& operator=(ShadowMap&& other) noexcept;

		void BindToBuffer() const;
		void UnbindFromBuffer() const;

		void BindToTexture(std::size_t textureUnit) const;

		const unsigned& id;

	private:
		Vector2 m_Resolution;
		unsigned m_FrameBufferHandle;
		unsigned m_DepthMapHandle;
	};
}
