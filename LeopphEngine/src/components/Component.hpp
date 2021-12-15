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
			Component(const Component& other) = default;
			Component& operator=(const Component& other) = default;

			Component(Component&& other) = default;
			Component& operator=(Component&& other) = default;

			LEOPPHAPI virtual ~Component() = default;

			// The Entity the Component is attached to.
			LEOPPHAPI Entity* Entity() const;

		protected:
			LEOPPHAPI explicit Component(leopph::Entity* entity);

		private:
			leopph::Entity* m_Entity;
	};
}
