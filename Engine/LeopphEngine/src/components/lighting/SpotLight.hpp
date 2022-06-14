#pragma once

#include "AttenuatedLight.hpp"


namespace leopph
{
	// SpotLights are special Lights that shine in a cone.
	// They have position, orientation, attenuation, and radius.
	class SpotLight final : public AttenuatedLight
	{
		public:
			// Get the angle in degrees at which the light starts to fade out.
			[[nodiscard]] LEOPPHAPI
			auto InnerAngle() const noexcept -> float;

			// Set the angle in degrees at which the light starts to fade out.
			LEOPPHAPI
			auto InnerAngle(float degrees) noexcept -> void;

			// Get the angle in degrees at which the light is completely cut.
			[[nodiscard]] LEOPPHAPI
			auto OuterAngle() const noexcept -> float;

			// Set the angle in degrees at which the light is completely cut.
			LEOPPHAPI
			auto OuterAngle(float degrees) noexcept -> void;

			LEOPPHAPI
			auto Owner(Entity* entity) -> void override;
			using AttenuatedLight::Owner;

			LEOPPHAPI
			auto Active(bool active) -> void override;
			using AttenuatedLight::Active;

			SpotLight() = default;

			SpotLight(SpotLight const& other) = default;

			LEOPPHAPI
			auto operator=(SpotLight const& other) -> SpotLight&;

			[[nodiscard]] LEOPPHAPI
			auto Clone() const -> ComponentPtr<> override;

			SpotLight(SpotLight&& other) = delete;
			auto operator=(SpotLight&& other) -> void = delete;

			LEOPPHAPI ~SpotLight() override;

		private:
			float m_InnerAngle{30.f};
			float m_OuterAngle{30.f};
	};
}
