#include "Light.hpp"


namespace leopph::internal
{
	Light::Light(leopph::Entity* const entity) :
		Component{entity},
		m_CastsShadow{false},
		m_Range{1000.f},
		m_Diffuse{1.0f, 1.0f, 1.0f},
		m_Specular{1.0f, 1.0f, 1.0f}
	{}

	Light::~Light() = default;

	auto Light::Diffuse() const -> const Vector3&
	{
		return m_Diffuse;
	}

	auto Light::Diffuse(const Vector3& value) -> void
	{
		m_Diffuse = value;
	}

	auto Light::Specular() const -> const Vector3&
	{
		return m_Specular;
	}

	auto Light::Specular(const Vector3& value) -> void
	{
		m_Specular = value;
	}

	auto Light::Range() const -> float
	{
		return m_Range;
	}

	auto Light::Range(const float value) -> void
	{
		m_Range = value;
	}

	auto Light::CastsShadow() const -> bool
	{
		return m_CastsShadow;
	}

	auto Light::CastsShadow(const bool value) -> void
	{
		m_CastsShadow = value;
	}
}
