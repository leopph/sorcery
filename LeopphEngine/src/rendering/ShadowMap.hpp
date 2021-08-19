#pragma once

#include "../math/vector.h"

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

		const unsigned& id;

	private:
		unsigned m_FrameBufferHandle;
		unsigned m_DepthMapHandle;
	};
}
