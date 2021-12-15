#pragma once

#include "../api/LeopphApi.hpp"


namespace leopph
{
	class Entity;


	/* Components are objects that are attached to Entities to provide additional functionality.
	 * They serve as a form of decoration to Entities and define their properties.
	 * Subclass this to provide your own properties to Entities. */
	class Component
	{
	public:
		LEOPPHAPI explicit Component(Entity& owner);

		Component(const Component&) = delete;
		void operator=(const Component&) = delete;

		Component(Component&&) = delete;
		void operator=(Component&&) = delete;

		LEOPPHAPI virtual ~Component() = 0;

		// The Entity the Component is attached to.
		Entity& Entity;
	};
}
