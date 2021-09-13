#include "DirLight.hpp"

#include "../../data/DataManager.hpp"

namespace leopph
{
	DirectionalLight::DirectionalLight(Entity& owner) :
		Light{ owner }
	{
		impl::DataManager::DirectionalLight(this);
	}

	DirectionalLight::~DirectionalLight()
	{
		impl::DataManager::DirectionalLight(nullptr);
	}

	const Vector3& DirectionalLight::Direction() const
	{
		return entity.Transform->Forward();
	}
}