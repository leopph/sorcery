#pragma once

#include "Light.hpp"


namespace leopph
{
	// Attenuated lights are special Lights whose intensity decreases by distance.
	class AttenuatedLight : public Light
	{
		public:
			// Get the attenuations constanst value.
			[[nodiscard]] constexpr auto Constant() const noexcept;

			// Get the attenuation's linear value.
			// This scales linearly with distance.
			[[nodiscard]] constexpr auto Linear() const noexcept;

			// Get the attenuation's quadratic value.
			// This scales quadratically with distance.
			[[nodiscard]] constexpr auto Quadratic() const noexcept;

			// Get the distance where the light effect fully cuts off.
			[[nodiscard]] constexpr auto Range() const noexcept;

			// Set the attenuation's constanst value.
			constexpr auto Constant(float value) noexcept;

			// Set the attenuation's linear value.
			// This scales linearly with distance.
			constexpr auto Linear(float value) noexcept;

			// Set the attenuation's quadratic value.
			// This scales quadratically with distance.
			constexpr auto Quadratic(float value) noexcept;

			// Set the distance where the light effect fully cuts off.
			constexpr auto Range(float value) noexcept;

		protected:
			using Light::Light;
			using Light::operator=;

		private:
			float m_Constant{1.0f};
			float m_Linear{0.14f};
			float m_Quadratic{0.07f};
			float m_Range{32.f};
	};


	constexpr auto AttenuatedLight::Constant() const noexcept
	{
		return m_Constant;
	}


	constexpr auto AttenuatedLight::Constant(const float value) noexcept
	{
		m_Constant = value;
	}


	constexpr auto AttenuatedLight::Linear() const noexcept
	{
		return m_Linear;
	}


	constexpr auto AttenuatedLight::Linear(const float value) noexcept
	{
		m_Linear = value;
	}


	constexpr auto AttenuatedLight::Quadratic() const noexcept
	{
		return m_Quadratic;
	}


	constexpr auto AttenuatedLight::Quadratic(const float value) noexcept
	{
		m_Quadratic = value;
	}


	constexpr auto AttenuatedLight::Range() const noexcept
	{
		return m_Range;
	}


	constexpr auto AttenuatedLight::Range(const float value) noexcept
	{
		m_Range = value;
	}
}
