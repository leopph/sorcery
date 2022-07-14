#pragma once

#include "AttenuatedLight.hpp"
#include "Types.hpp"


namespace leopph
{
	// SpotLights are special Lights that shine in a cone.
	// They have position, orientation, attenuation, and radius.
	class SpotLight final : public AttenuatedLight
	{
		public:
			// Get the angle in degrees at which the light starts to fade out.
			[[nodiscard]] auto LEOPPHAPI InnerAngle() const noexcept -> f32;

			// Set the angle in degrees at which the light starts to fade out.
			auto LEOPPHAPI InnerAngle(f32 degrees) noexcept -> void;


			// Get the angle in degrees at which the light is completely cut.
			[[nodiscard]] auto LEOPPHAPI OuterAngle() const noexcept -> f32;

			// Set the angle in degrees at which the light is completely cut.
			auto LEOPPHAPI OuterAngle(f32 degrees) noexcept -> void;

			
			auto LEOPPHAPI Owner(Entity* entity) -> void override;
			using AttenuatedLight::Owner;

			
			auto LEOPPHAPI Active(bool active) -> void override;
			using AttenuatedLight::Active;


			[[nodiscard]] auto LEOPPHAPI Clone() const -> ComponentPtr<> override;

			SpotLight() = default;

			SpotLight(SpotLight const& other) = default;
			auto LEOPPHAPI operator=(SpotLight const& other) -> SpotLight&;

			SpotLight(SpotLight&& other) = delete;
			auto operator=(SpotLight&& other) -> void = delete;

			LEOPPHAPI ~SpotLight() override;

		private:
			float m_InnerAngle{30.f};
			float m_OuterAngle{30.f};
	};
}
