#include "AmbientLight.hpp"

#include "../../instances/InstanceHolder.hpp"

leopph::AmbientLight& leopph::AmbientLight::Instance()
{
	auto ret = impl::InstanceHolder::AmbientLight();

	if (ret == nullptr)
	{
		impl::InstanceHolder::AmbientLight(new AmbientLight{});
		ret = impl::InstanceHolder::AmbientLight();
	}

	return *ret;
}

const leopph::Vector3& leopph::AmbientLight::Intensity() const
{
	return m_Intensity;
}

void leopph::AmbientLight::Intensity(const Vector3& newInt)
{
	m_Intensity = newInt;
}

leopph::AmbientLight::AmbientLight() :
	m_Intensity{ 0.5f, 0.5f, 0.5f }
{}