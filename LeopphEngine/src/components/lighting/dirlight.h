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
		/* Internally used constructor */
		LEOPPHAPI DirectionalLight(leopph::Object& object);

		/* The lighing direction of the component */
		LEOPPHAPI void Direction(const Vector3& newDir);
		LEOPPHAPI const Vector3& Direction() const;

	private:
		Vector3 m_Direction{ 1.0f, 1.0f, 1.0f };
	};
}