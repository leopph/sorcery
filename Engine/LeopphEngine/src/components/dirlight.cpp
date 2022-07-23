#include "DirLight.hpp"

#include "Math.hpp"
#include "../InternalContext.hpp"
#include "../data/DataManager.hpp"


namespace leopph
{
	Vector3 const& DirectionalLight::Direction() const noexcept
	{
		return Component::Owner()->get_transform().get_forward_axis();
	}



	f32 DirectionalLight::ShadowExtension() const noexcept
	{
		return m_ShadowRange;
	}



	void DirectionalLight::ShadowExtension(f32 const newRange)
	{
		m_ShadowRange = math::Clamp(newRange, 0, math::Abs(newRange));
	}



	void DirectionalLight::Owner(Entity* entity)
	{
		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActiveDirLight(this);
		}

		Light::Owner(entity);

		if (InUse())
		{
			dataManager->RegisterActiveDirLight(this);
		}
	}



	void DirectionalLight::Active(bool const active)
	{
		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActiveDirLight(this);
		}

		Light::Active(active);

		if (InUse())
		{
			dataManager->RegisterActiveDirLight(this);
		}
	}



	ComponentPtr<> DirectionalLight::Clone() const
	{
		return CreateComponent<DirectionalLight>(*this);
	}



	DirectionalLight& DirectionalLight::operator=(DirectionalLight const& other)
	{
		if (this == &other)
		{
			return *this;
		}

		auto* const dataManager = internal::GetDataManager();

		if (InUse())
		{
			dataManager->UnregisterActiveDirLight(this);
		}

		Light::operator=(other);
		m_ShadowRange = other.m_ShadowRange;

		if (InUse())
		{
			dataManager->RegisterActiveDirLight(this);
		}

		return *this;
	}



	DirectionalLight::~DirectionalLight()
	{
		internal::GetDataManager()->UnregisterActiveDirLight(this);
	}
}
