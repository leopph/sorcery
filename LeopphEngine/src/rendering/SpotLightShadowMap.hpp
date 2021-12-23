#pragma once

#include "../events/SpotShadowResolutionEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "shaders/ShaderProgram.hpp"

#include <cstddef>



namespace leopph::internal
{
	class SpotLightShadowMap final : EventReceiver<SpotShadowResolutionEvent>
	{
	public:
		SpotLightShadowMap();

		SpotLightShadowMap(const SpotLightShadowMap&) = delete;
		SpotLightShadowMap(SpotLightShadowMap&& other) noexcept;

		~SpotLightShadowMap() override;

		void operator=(const SpotLightShadowMap&) = delete;
		SpotLightShadowMap& operator=(SpotLightShadowMap&& other) noexcept;

		void BindForWriting() const;
		void UnbindFromWriting() const;

		[[nodiscard]] int BindForReading(ShaderProgram& shader, int textureUnit) const;
		void UnbindFromReading() const;

		void Clear() const;

		const unsigned& Id;


	private:
		void Init();
		void Deinit() const;
		void OnEventReceived(EventParamType event) override;

		unsigned m_Fbo;
		unsigned m_DepthTex;
		mutable int m_CurrentBindIndex;
		std::size_t m_Resolution;
	};
}