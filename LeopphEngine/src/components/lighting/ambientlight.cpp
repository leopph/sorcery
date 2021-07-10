#include "ambientlight.h"

#include "../../instances/instanceholder.h"

#include <memory>
#include <utility>

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

void leopph::AmbientLight::Intensity(Vector3 newColor)
{
	m_Intensity = std::move(newColor);
}

leopph::AmbientLight::AmbientLight() :
	m_Intensity{ 0.5f, 0.5f, 0.5f }
{}