#include "PointLight.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	PointLight::PointLight(leopph::Entity* const entity) :
		AttenuatedLight{entity}
	{
		impl::DataManager::Register(this);
	}

	PointLight::~PointLight()
	{
		impl::DataManager::Unregister(this);
	}
}
