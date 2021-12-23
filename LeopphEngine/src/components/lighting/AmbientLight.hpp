#pragma once

#include "../../api/LeopphApi.hpp"
#include "../../math/Vector.hpp"


namespace leopph
{
	/* The AmbientLight class provides your scene with a global ambient light level.
	 * There is always exaclty one of it in a scene.
	 * Ambient lights do not provied shadow or specular highlights. */
	class AmbientLight final
	{
		public:
			// Get the AmbientLight instance.
			LEOPPHAPI static auto Instance() -> AmbientLight&;

			/* Get the current color values the AmbientLight uses to light objects.
			 * Component values are in the [0; 1] range. */
			[[nodiscard]]
			LEOPPHAPI auto Intensity() const -> const Vector3&;

			/* Get the current color values the AmbientLight uses to light objects.
			 * Component values must be in the [0; 1] range. */
			LEOPPHAPI auto Intensity(const Vector3& newInt) -> void;

			AmbientLight(const AmbientLight& other) = delete;
			auto operator=(const AmbientLight& other) -> AmbientLight& = delete;

			AmbientLight(AmbientLight&& other) = delete;
			auto operator=(AmbientLight&& other) -> AmbientLight& = delete;

			~AmbientLight() = default;

		private:
			AmbientLight();

			Vector3 m_Intensity;
	};
}
