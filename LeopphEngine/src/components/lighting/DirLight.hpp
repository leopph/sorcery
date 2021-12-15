#pragma once

#include "Light.hpp"
#include "../../api/LeopphApi.hpp"
#include "../../math/Vector.hpp"


namespace leopph
{
	/* DirectionalLights are special Lights that provide a way to light your scene with a source that appears to be infinitely far.
	 * The Position of the DirectionalLight does not matter, it always affects things from the direction it is set to. */
	class DirectionalLight final : public impl::Light
	{
		public:
			/* The direction where the DirectionalLight shines.
			 * This is exactly the same as the owning Entity's forward, or Z vector. */
			LEOPPHAPI const Vector3& Direction() const;

			LEOPPHAPI explicit DirectionalLight(leopph::Entity* entity);

			DirectionalLight(const DirectionalLight&) = delete;
			void operator=(const DirectionalLight&) = delete;

			DirectionalLight(DirectionalLight&&) = delete;
			void operator=(DirectionalLight&&) = delete;

			LEOPPHAPI ~DirectionalLight() override;
	};
}
