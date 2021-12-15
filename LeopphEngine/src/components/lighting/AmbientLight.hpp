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
			LEOPPHAPI static AmbientLight& Instance();

			/* Get the current color values the AmbientLight uses to light objects.
			 * Component values are in the [0; 1] range. */
			[[nodiscard]]
			LEOPPHAPI const Vector3& Intensity() const;

			/* Get the current color values the AmbientLight uses to light objects.
			 * Component values must be in the [0; 1] range. */
			LEOPPHAPI void Intensity(const Vector3& newInt);


			AmbientLight(const AmbientLight& other) = delete;
			AmbientLight& operator=(const AmbientLight& other) = delete;

			AmbientLight(AmbientLight&& other) = delete;
			AmbientLight& operator=(AmbientLight&& other) = delete;

			~AmbientLight() = default;


		private:
			AmbientLight();

			Vector3 m_Intensity;
	};
}
