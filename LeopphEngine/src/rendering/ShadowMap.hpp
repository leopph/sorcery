#pragma once

#include "../math/Vector.hpp"

namespace leopph::impl
{
	class ShadowMap
	{
	public:
		explicit ShadowMap(const Vector2& resolution);

		ShadowMap(const ShadowMap&) = delete;
		ShadowMap(ShadowMap&& other) noexcept;

		void operator=(const ShadowMap&) = delete;
		ShadowMap& operator=(ShadowMap&& other) noexcept;

		~ShadowMap();

		void BindForWriting() const;
		void UnbindFromWriting() const;

		[[nodiscard]] int BindForReading(int textureUnit) const;
		void UnbindFromReading() const;

		void Clear() const;

		const unsigned& Id;

	private:
		Vector2 m_Resolution;
		unsigned m_Fbo;
		unsigned m_DepthTex;
		mutable int m_CurrentBindIndex;
	};
}
