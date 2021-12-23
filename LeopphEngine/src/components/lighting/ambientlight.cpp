#include "AmbientLight.hpp"

#include "../../data/DataManager.hpp"

auto leopph::AmbientLight::Instance() -> leopph::AmbientLight&
{
	static AmbientLight instance;
	return instance;
}

auto leopph::AmbientLight::Intensity() const -> const leopph::Vector3&
{
	return m_Intensity;
}

auto leopph::AmbientLight::Intensity(const Vector3& newInt) -> void
{
	m_Intensity = newInt;
}

leopph::AmbientLight::AmbientLight() :
	m_Intensity{0.5f, 0.5f, 0.5f}
{}
