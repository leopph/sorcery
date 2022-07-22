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
			[[nodiscard]] LEOPPHAPI f32 InnerAngle() const noexcept;

			// Set the angle in degrees at which the light starts to fade out.
			LEOPPHAPI void InnerAngle(f32 degrees) noexcept;


			// Get the angle in degrees at which the light is completely cut.
			[[nodiscard]] LEOPPHAPI f32 OuterAngle() const noexcept;

			// Set the angle in degrees at which the light is completely cut.
			LEOPPHAPI void OuterAngle(f32 degrees) noexcept;


			LEOPPHAPI void Owner(Entity* entity) override;
			using AttenuatedLight::Owner;


			LEOPPHAPI void Active(bool active) override;
			using AttenuatedLight::Active;


			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;

			SpotLight() = default;

			SpotLight(SpotLight const& other) = default;
			LEOPPHAPI SpotLight& operator=(SpotLight const& other);

			SpotLight(SpotLight&& other) = delete;
			void operator=(SpotLight&& other) = delete;

			LEOPPHAPI ~SpotLight() override;

		private:
			float m_InnerAngle{30.f};
			float m_OuterAngle{30.f};
	};
}
