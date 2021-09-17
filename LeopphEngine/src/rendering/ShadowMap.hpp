#pragma once

#include <cstddef>


namespace leopph::impl
{
	class ShadowMap
	{
		public:
			explicit ShadowMap(std::size_t resolution);

			ShadowMap(const ShadowMap&) = delete;
			ShadowMap(ShadowMap&& other) noexcept;

			void operator=(const ShadowMap&) = delete;
			ShadowMap& operator=(ShadowMap&& other) noexcept;

			virtual ~ShadowMap();

			void BindForWriting() const;
			void UnbindFromWriting() const;

			[[nodiscard]] int BindForReading(int textureUnit) const;
			void UnbindFromReading() const;

			void Clear() const;

			const unsigned& Id;


		protected:
			std::size_t m_Resolution;
			void Init();
			void Deinit() const;


		private:
			unsigned m_Fbo;
			unsigned m_DepthTex;
			mutable int m_CurrentBindIndex;
	};
}
