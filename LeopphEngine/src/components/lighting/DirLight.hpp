#pragma once

#include "../../api/leopphapi.h"
#include "Light.hpp"
#include "../../math/Vector.hpp"

namespace leopph
{
	class Entity;

	/*-----------------------------------------------------------------------------------------------------------------------
	Directional lights are components that provide a way to light your scene with a source that appears to be infinitely far.
	The position of the light source does not matter, it always lights things from the direction it is set to.
	See "Light.hpp" and "Component.hpp" for more information.
	-----------------------------------------------------------------------------------------------------------------------*/
	class DirectionalLight final : public impl::Light
	{
	public:
		LEOPPHAPI explicit DirectionalLight(Entity& owner);
		LEOPPHAPI ~DirectionalLight() override;

		DirectionalLight(const DirectionalLight&) = delete;
		DirectionalLight(DirectionalLight&&) = delete;
		void operator=(const DirectionalLight&) = delete;
		void operator=(DirectionalLight&&) = delete;

		/* The direction of the Light. This is exactly the same
		as the forward vector of the Entity's Transform that the
		DirectionalLight is attached to. */
		LEOPPHAPI const Vector3& Direction() const;
	};
}