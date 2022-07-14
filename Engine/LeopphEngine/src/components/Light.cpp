#include "Light.hpp"


namespace leopph
{
	auto Light::Diffuse() const noexcept -> Vector3 const&
	{
		return m_Diffuse;
	}


	auto Light::Diffuse(Vector3 const& value) noexcept -> void
	{
		m_Diffuse = value;
	}


	auto Light::Specular() const noexcept -> Vector3 const&
	{
		return m_Specular;
	}


	auto Light::Specular(Vector3 const& value) noexcept -> void
	{
		m_Specular = value;
	}


	auto Light::CastsShadow() const noexcept -> bool
	{
		return m_CastsShadow;
	}


	auto Light::CastsShadow(bool const value) noexcept -> void
	{
		m_CastsShadow = value;
	}
}
