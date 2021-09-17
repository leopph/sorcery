#include "SpotLightShadowMap.hpp"

#include "../config/Settings.hpp"


namespace leopph::impl
{
	SpotLightShadowMap::SpotLightShadowMap() :
		ShadowMap{Settings::SpotLightShadowMapResolution()}
	{}


	int SpotLightShadowMap::BindForReading(const DeferredSpotLightShader& shader, const int textureUnit) const
	{
		const auto ret{ShadowMap::BindForReading(textureUnit)};
		shader.SetShadowMap(ret);
		return ret;
	}


	void SpotLightShadowMap::OnEventReceived(EventParamType event)
	{
		m_Resolution = event.Resolution;
		Deinit();
		Init();
	}

}