#pragma once

#include "../../api/LeopphApi.hpp"
#include "../../math/Vector.hpp"


namespace leopph
{
	// The AmbientLight class provides scenes with a global ambient light level.
	// There is always exaclty one of it in a scene.
	// Ambient lights do not provide shadow or specular highlights.
	class AmbientLight final
	{
		public:
			// Get the AmbientLight instance.
			LEOPPHAPI static auto Instance() -> AmbientLight&;

			// Get the current color values the AmbientLight uses to light objects.
			// Component values are in the [0; 1] range.
			[[nodiscard]] constexpr auto Intensity() const noexcept -> const auto&;

			// Set the color values the AmbientLight uses to light objects.
			// Component values must be in the [0; 1] range.
			constexpr auto Intensity(const Vector3& newInt) noexcept;

			AmbientLight(const AmbientLight& other) = delete;
			auto operator=(const AmbientLight& other) -> AmbientLight& = delete;

			AmbientLight(AmbientLight&& other) = delete;
			auto operator=(AmbientLight&& other) -> AmbientLight& = delete;

			~AmbientLight() = default;

		private:
			AmbientLight() = default;

			Vector3 m_Intensity{0.5f, 0.5f, 0.5f};
	};


	constexpr auto AmbientLight::Intensity() const noexcept -> const auto&
	{
		return m_Intensity;
	}


	constexpr auto AmbientLight::Intensity(const Vector3& newInt) noexcept
	{
		m_Intensity = newInt;
	}

}
