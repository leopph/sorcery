#include "DirLight.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	DirectionalLight::DirectionalLight(leopph::Entity* const entity) :
		Light{entity}
	{
		impl::DataManager::Instance().DirectionalLight(this);
	}

	DirectionalLight::~DirectionalLight()
	{
		impl::DataManager::Instance().DirectionalLight(nullptr);
	}

	const Vector3& DirectionalLight::Direction() const
	{
		return Entity()->Transform()->Forward();
	}
}
