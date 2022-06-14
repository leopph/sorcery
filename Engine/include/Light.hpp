#pragma once

#include "Vector.hpp"
#include "Component.hpp"


namespace leopph
{
	// Base class for all lights.
	class Light : public Component
	{
		public:
			// Get the current diffuse intensity and color.
			// This value is in the [0; 1] range per component.
			[[nodiscard]] constexpr
			auto Diffuse() const noexcept -> auto&;

			// Get the current specular highlight intensity and color.
			// This value is in the [0; 1] range per component.
			[[nodiscard]] constexpr
			auto Specular() const -> auto&;

			// Get whether the Light's effect can be occluded by objects.
			// This only works if objects have this property set to true.
			// This value is false by default.
			[[nodiscard]] constexpr
			auto CastsShadow() const noexcept;

			// Set the current diffuse intensity and color.
			// This value must be in the [0; 1] range per component.
			constexpr
			auto Diffuse(Vector3 const& value);

			// Set the current specular highlight intensity and color.
			// This value must be in the [0; 1] range per component.
			constexpr
			auto Specular(Vector3 const& value) noexcept;

			// Set whether the Light's effect can be occluded by objects.
			// This only works if objects have this property set to true.
			// This value is false by default.
			constexpr
			auto CastsShadow(bool value) noexcept;

		protected:
			using Component::Component;
			using Component::operator=;

		private:
			bool m_CastsShadow{false};
			Vector3 m_Diffuse{1.f, 1.f, 1.f};
			Vector3 m_Specular{1.f, 1.f, 1.f};
	};


	constexpr auto Light::Diffuse() const noexcept -> auto&
	{
		return m_Diffuse;
	}


	constexpr auto Light::Specular() const -> auto&
	{
		return m_Specular;
	}


	constexpr auto Light::CastsShadow() const noexcept
	{
		return m_CastsShadow;
	}


	constexpr auto Light::Diffuse(Vector3 const& value)
	{
		m_Diffuse = value;
	}


	constexpr auto Light::Specular(Vector3 const& value) noexcept
	{
		m_Specular = value;
	}


	constexpr auto Light::CastsShadow(bool const value) noexcept
	{
		m_CastsShadow = value;
	}
}
