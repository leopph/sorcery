#include "PointLight.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	PointLight::PointLight(leopph::Entity* const entity) :
		AttenuatedLight{entity}
	{
		internal::DataManager::Instance().RegisterPointLight(this);
	}

	PointLight::~PointLight()
	{
		internal::DataManager::Instance().UnregisterPointLight(this);
	}
}
