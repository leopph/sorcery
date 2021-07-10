#pragma once

#include "../../api/leopphapi.h"

#include "../../math/vector.h"

namespace leopph
{
	/*---------------------------------------------------------------------------
	The AmbientLight class provides your scene with a global ambient light level.
	There is only one of it per scene. 
	---------------------------------------------------------------------------*/
	class AmbientLight
	{
	public:
		/* The only instance of AmbientLight */
		LEOPPHAPI static AmbientLight& Instance();

		/* Intensity on different color channels.
		Values are in range [0, 1]. */
		LEOPPHAPI const Vector3& Intensity() const;
		LEOPPHAPI void Intensity(Vector3 newInt);

	private:
		AmbientLight();

		Vector3 m_Intensity;
	};
}