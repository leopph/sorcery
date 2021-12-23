#include "DirLight.hpp"

#include "../../data/DataManager.hpp"


namespace leopph
{
	DirectionalLight::DirectionalLight(leopph::Entity* const entity) :
		Light{entity}
	{
		internal::DataManager::Instance().DirectionalLight(this);
	}

	DirectionalLight::~DirectionalLight()
	{
		internal::DataManager::Instance().DirectionalLight(nullptr);
	}

	auto DirectionalLight::Direction() const -> const Vector3&
	{
		return Entity()->Transform()->Forward();
	}
}
