#pragma once

#include "AttenuatedLight.hpp"


namespace leopph
{
	// SpotLights are special Lights that shine in a cone.
	// They have position, orientation, attenuation, and radius.
	class SpotLight final : public internal::AttenuatedLight
	{
		public:
			// Get the angle in degrees at which the light starts to fade out.
			[[nodiscard]] constexpr auto InnerAngle() const noexcept;

			// Get the angle in degrees at which the light is completely cut.
			[[nodiscard]] constexpr auto OuterAngle() const noexcept;

			// Set the angle in degrees at which the light starts to fade out.
			constexpr auto InnerAngle(float degrees) noexcept;

			// Set the angle in degrees at which the light is completely cut.
			constexpr auto OuterAngle(float degrees) noexcept;

			LEOPPHAPI auto Activate() -> void override;
			LEOPPHAPI auto Deactivate() -> void override;

			LEOPPHAPI explicit SpotLight(leopph::Entity* entity);

			SpotLight(const SpotLight&) = delete;
			auto operator=(const SpotLight&) -> void = delete;

			SpotLight(SpotLight&&) = delete;
			auto operator=(SpotLight&&) -> void = delete;

			LEOPPHAPI ~SpotLight() override;

		private:
			float m_InnerAngle{30.f};
			float m_OuterAngle{30.f};
	};


	constexpr auto SpotLight::InnerAngle() const noexcept
	{
		return m_InnerAngle;
	}


	constexpr auto SpotLight::OuterAngle() const noexcept
	{
		return m_OuterAngle;
	}


	constexpr auto SpotLight::InnerAngle(const float degrees) noexcept
	{
		m_InnerAngle = degrees;
	}


	constexpr auto SpotLight::OuterAngle(const float degrees) noexcept
	{
		m_OuterAngle = degrees;
	}
}
