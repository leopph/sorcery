#include "SpotLight.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	SpotLight::SpotLight(leopph::Entity* const entity) :
		AttenuatedLight{entity}
	{
		internal::DataManager::Instance().RegisterSpotLight(this);
	}

	SpotLight::~SpotLight()
	{
		internal::DataManager::Instance().UnregisterSpotLight(this);
	}
}
