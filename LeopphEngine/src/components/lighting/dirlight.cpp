#include "DirLight.hpp"

#include "../../data/DataManager.hpp"
#include "../../math/Math.hpp"


namespace leopph
{
	auto DirectionalLight::ShadowExtension(const float newRange) -> void
	{
		m_ShadowRange = math::Clamp(newRange, 0, math::Abs(newRange));
	}


	auto DirectionalLight::Activate() -> void
	{
		if (IsActive())
		{
			return;
		}

		Light::Activate();

		if (IsAttached())
		{
			auto& dataManager{internal::DataManager::Instance()};
			dataManager.UnregisterDirLight(this, false);
			dataManager.RegisterDirLight(this, true);
		}
	}


	auto DirectionalLight::Deactivate() -> void
	{
		if (!IsActive())
		{
			return;
		}

		Light::Deactivate();

		if (IsAttached())
		{
			auto& dataManager{internal::DataManager::Instance()};
			dataManager.UnregisterDirLight(this, true);
			dataManager.RegisterDirLight(this, false);
		}
	}


	auto DirectionalLight::Attach(leopph::Entity* entity) -> void
	{
		if (IsAttached())
		{
			return;
		}

		Light::Attach(entity);

		internal::DataManager::Instance().RegisterDirLight(this, IsActive());
	}


	auto DirectionalLight::Detach() -> void
	{
		if (!IsAttached())
		{
			return;
		}

		Light::Detach();

		internal::DataManager::Instance().UnregisterDirLight(this, IsActive());
	}


	DirectionalLight::~DirectionalLight()
	{
		Detach();
	}
}
