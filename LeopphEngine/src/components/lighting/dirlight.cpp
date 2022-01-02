#include "DirLight.hpp"

#include "../../data/DataManager.hpp"
#include "../../math/LeopphMath.hpp"


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


	auto DirectionalLight::ShadowExtension(const float newRange)
	{
		m_ShadowRange = math::Clamp(newRange, 0, math::Abs(newRange));
	}
}
