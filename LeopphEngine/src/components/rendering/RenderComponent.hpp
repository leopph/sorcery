#pragma once

#include "../Component.hpp"
#include "../../api/LeopphApi.hpp"


namespace leopph::impl
{
	// The RenderComponent class provides a base for all Components related to rendering objects.
	class RenderComponent : public Component
	{
	public:
		using Component::Component;

		LEOPPHAPI ~RenderComponent() override = default;;

		RenderComponent& operator=(const RenderComponent& other) = delete;
		RenderComponent& operator=(RenderComponent&& other) = delete;

		/* Get whether the rendered object occludes light from other objects.
		 * This only works if the Light used also has this property set to true.
		 * This value is false by default. */
		[[nodiscard]]
		virtual bool CastsShadow() const = 0;

		/* Set whether the rendered object occludes light from other objects.
		 * This only works if the Light used also has this property set to true.
		 * This value is false by default. */
		virtual void CastsShadow (bool value) = 0;
	};
}