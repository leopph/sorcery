#include "PointLight.hpp"

#include "../../data/DataManager.hpp"

namespace leopph
{
	PointLight::PointLight(Object& owner) :
		AttenuatedLight{ owner }
	{
		impl::DataManager::Register(this);
	}

	PointLight::~PointLight()
	{
		impl::DataManager::Unregister(this);
	}

}