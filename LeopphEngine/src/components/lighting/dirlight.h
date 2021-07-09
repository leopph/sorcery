#pragma once

#include "../../api/leopphapi.h"
#include "light.h"
#include "../../math/vector.h"

namespace leopph
{
	/*-----------------------------------------------------------------------------------------------------------------------
	Directional lights are components that provide a way to light your scene with a source that appears to be infinitely far.
	The position of the light source does not matter, it always lights the objects from the direction it is set to.
	See "component.h" for more information.
	-----------------------------------------------------------------------------------------------------------------------*/

	class DirectionalLight : public impl::Light
	{
	public:
		/* Internally used functions */
		LEOPPHAPI DirectionalLight();
		LEOPPHAPI ~DirectionalLight() override;

		/* The direction of the Light. This is exactly the same
		as the forward vector of the Object that the
		DirectionalLight is attached to. */
		LEOPPHAPI const Vector3& Direction() const;
	};
}