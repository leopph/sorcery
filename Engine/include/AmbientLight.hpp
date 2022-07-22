#pragma once

#include "LeopphApi.hpp"
#include "Vector.hpp"


namespace leopph
{
	// The AmbientLight class provides scenes with a global ambient light level.
	// There is always exaclty one of it in a scene.
	// Ambient lights do not provide shadow or specular highlights.
	class AmbientLight final
	{
		public:
			// Get the AmbientLight instance.
			LEOPPHAPI static AmbientLight& Instance();

			// Get the current color values the AmbientLight uses to light objects.
			// Component values are in the [0; 1] range.
			[[nodiscard]] constexpr const auto& Intensity() const noexcept;

			// Set the color values the AmbientLight uses to light objects.
			// Component values must be in the [0; 1] range.
			constexpr auto Intensity(Vector3 const& newInt) noexcept;

			AmbientLight(AmbientLight const& other) = delete;
			AmbientLight& operator=(AmbientLight const& other) = delete;

			AmbientLight(AmbientLight&& other) = delete;
			AmbientLight& operator=(AmbientLight&& other) = delete;

			~AmbientLight() = default;

		private:
			AmbientLight() = default;

			Vector3 m_Intensity{0.1f, 0.1f, 0.1f};
	};


	constexpr const auto& AmbientLight::Intensity() const noexcept
	{
		return m_Intensity;
	}


	constexpr auto AmbientLight::Intensity(Vector3 const& newInt) noexcept
	{
		m_Intensity = newInt;
	}
}
