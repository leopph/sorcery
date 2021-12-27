#pragma once

#include "../Component.hpp"
#include "../../math/Vector.hpp"


namespace leopph::internal
{
	// Base class for all lights.
	class Light : public Component
	{
		public:
			// Get the current diffuse intensity and color.
			// This value is in the [0; 1] range per component.
			[[nodiscard]] constexpr auto Diffuse() const noexcept -> auto&;

			// Get the current specular highlight intensity and color.
			// This value is in the [0; 1] range per component.
			[[nodiscard]] constexpr auto Specular() const -> auto&;

			// Get whether the Light's effect can be occluded by objects.
			// This only works if objects have this property set to true.
			// This value is false by default.
			[[nodiscard]] constexpr auto CastsShadow() const noexcept;

			// Get the distance where the Light's effect fully cuts off.
			[[nodiscard]] constexpr auto Range() const noexcept;

			// Set the current diffuse intensity and color.
			// This value must be in the [0; 1] range per component.
			constexpr auto Diffuse(const Vector3& value);

			// Set the current specular highlight intensity and color.
			// This value must be in the [0; 1] range per component.
			constexpr auto Specular(const Vector3& value) noexcept;

			// Set whether the Light's effect can be occluded by objects.
			// This only works if objects have this property set to true.
			// This value is false by default.
			constexpr auto CastsShadow(bool value) noexcept;

			// Set the distance where the Light's effect fully cuts off.
			constexpr auto Range(float value) noexcept;

		protected:
			using Component::Component;
			using Component::operator=;

		private:
			bool m_CastsShadow{false};
			float m_Range{100.f};
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


	constexpr auto Light::Range() const noexcept
	{
		return m_Range;
	}


	constexpr auto Light::Diffuse(const Vector3& value)
	{
		m_Diffuse = value;
	}


	constexpr auto Light::Specular(const Vector3& value) noexcept
	{
		m_Specular = value;
	}


	constexpr auto Light::CastsShadow(const bool value) noexcept
	{
		m_CastsShadow = value;
	}


	constexpr auto Light::Range(const float value) noexcept
	{
		m_Range = value;
	}
}
