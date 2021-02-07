#include "light.h"

namespace leopph::implementation
{
	// destructor
	Light::~Light() {}


	// init static members
	std::set<Light*> Light::s_PointLights{};
	Light* Light::s_DirectionalLight{};


	// getters
	const std::set<Light*>& Light::PointLights()
	{
		return s_PointLights;
	}

	Light* Light::DirectionalLight()
	{
		return s_DirectionalLight;
	}



	// registration
	void Light::RegisterDirectionalLight(Light* light)
	{
		s_DirectionalLight = light;
	}

	void Light::RegisterPointLight(Light* light)
	{
		s_PointLights.insert(light);
	}


	void Light::UnregisterDirectionalLight(Light* light)
	{
		s_DirectionalLight = nullptr;
	}

	void Light::UnregisterPointLight(Light* light)
	{
		s_PointLights.erase(light);
	}


	const Vector3& Light::Ambient() const
	{
		return m_Ambient;
	}

	const Vector3& Light::Diffuse() const
	{
		return m_Diffuse;
	}

	const Vector3& Light::Specular() const
	{
		return m_Specular;
	}

	void Light::Ambient(const Vector3& value)
	{
		m_Ambient = value;
	}

	void Light::Diffuse(const Vector3& value)
	{
		m_Diffuse = value;
	}

	void Light::Specular(const Vector3& value)
	{
		m_Specular = value;
	}
}