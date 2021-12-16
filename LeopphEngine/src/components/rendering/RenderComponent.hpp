#pragma once

#include "../Component.hpp"


namespace leopph::impl
{
	class MeshDataGroup;


	// The RenderComponent class provides a base for all Components related to rendering objects.
	class RenderComponent : public Component
	{
		public:
			RenderComponent(leopph::Entity* entity, const MeshDataGroup& meshData);

			/* Get whether the rendered object occludes light from other objects.
			 * This only works if the Light used also has this property set to true.
			 * This value is false by default. */
			[[nodiscard]]
			virtual bool CastsShadow() const = 0;

			/* Set whether the rendered object occludes light from other objects.
			 * This only works if the Light used also has this property set to true.
			 * This value is false by default. */
			virtual void CastsShadow(bool value) = 0;


	};
}
