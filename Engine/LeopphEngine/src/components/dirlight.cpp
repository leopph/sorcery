#include "DirLight.hpp"

#include "../InternalContext.hpp"
#include "../data/DataManager.hpp"


namespace leopph
{
	Vector3 const& DirectionalLight::get_direction() const noexcept
	{
		return Component::Owner()->get_transform().get_forward_axis();
	}



	f32 DirectionalLight::get_shadow_near_plane() const noexcept
	{
		return mShadowNear;
	}



	void DirectionalLight::set_shadow_near_plane(f32 const newVal)
	{
		mShadowNear = newVal;
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
		mShadowNear = other.mShadowNear;

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
