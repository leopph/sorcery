#include "PointLight.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	PointLight::PointLight(leopph::Entity* const entity) :
		AttenuatedLight{entity}
	{
		impl::DataManager::Instance().RegisterPointLight(this);
	}

	PointLight::~PointLight()
	{
		impl::DataManager::Instance().UnregisterPointLight(this);
	}
}
