#include "DirLight.hpp"

#include "../../data/DataManager.hpp"
#include "../../math/LeopphMath.hpp"


namespace leopph
{
	auto DirectionalLight::Activate() -> void
	{
		if (IsActive())
		{
			return;
		}

		Light::Activate();
		auto& dataManager{internal::DataManager::Instance()};
		dataManager.UnregisterInactiveDirLight(this);
		dataManager.RegisterActiveDirLight(this);
	}


	auto DirectionalLight::Deactivate() -> void
	{
		if (!IsActive())
		{
			return;
		}

		Light::Deactivate();
		auto& dataManager{internal::DataManager::Instance()};
		dataManager.UnregisterActiveDirLight(this);
		dataManager.RegisterInactiveDirLight(this);
	}


	DirectionalLight::DirectionalLight(leopph::Entity* const entity) :
		Light{entity}
	{
		internal::DataManager::Instance().RegisterActiveDirLight(this);
	}


	DirectionalLight::~DirectionalLight()
	{
		if (IsActive())
		{
			internal::DataManager::Instance().UnregisterActiveDirLight(nullptr);
		}
		else
		{
			internal::DataManager::Instance().UnregisterInactiveDirLight(nullptr);
		}
	}


	auto DirectionalLight::ShadowExtension(const float newRange)
	{
		m_ShadowRange = math::Clamp(newRange, 0, math::Abs(newRange));
	}
}
