#include "DirLight.hpp"

#include "../../data/DataManager.hpp"
#include "../../math/Math.hpp"


namespace leopph
{
	auto DirectionalLight::Direction() const noexcept -> Vector3 const&
	{
		return Component::Owner()->Transform()->Forward();
	}


	auto DirectionalLight::ShadowExtension() const noexcept -> float
	{
		return m_ShadowRange;
	}


	auto DirectionalLight::ShadowExtension(float const newRange) -> void
	{
		m_ShadowRange = math::Clamp(newRange, 0, math::Abs(newRange));
	}


	auto DirectionalLight::Owner(Entity* entity) -> void
	{
		auto& dataManager = internal::DataManager::Instance();

		if (InUse())
		{
			dataManager.UnregisterActiveDirLight(this);
		}

		Light::Owner(entity);

		if (InUse())
		{
			dataManager.RegisterActiveDirLight(this);
		}
	}


	auto DirectionalLight::Active(bool const active) -> void
	{
		auto& dataManager = internal::DataManager::Instance();

		if (InUse())
		{
			dataManager.UnregisterActiveDirLight(this);
		}

		Light::Active(active);

		if (InUse())
		{
			dataManager.RegisterActiveDirLight(this);
		}
	}


	auto DirectionalLight::operator=(DirectionalLight const& other) -> DirectionalLight&
	{
		if (this == &other)
		{
			return *this;
		}

		auto& dataManager = internal::DataManager::Instance();

		if (InUse())
		{
			dataManager.UnregisterActiveDirLight(this);
		}

		Light::operator=(other);
		m_ShadowRange = other.m_ShadowRange;

		if (InUse())
		{
			dataManager.RegisterActiveDirLight(this);
		}

		return *this;
	}


	DirectionalLight::~DirectionalLight()
	{
		internal::DataManager::Instance().UnregisterActiveDirLight(this);
	}
}
