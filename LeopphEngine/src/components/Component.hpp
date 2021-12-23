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
			auto operator=(const Component& other) -> Component& = default;

			Component(Component&& other) = default;
			auto operator=(Component&& other) -> Component& = default;

			LEOPPHAPI virtual ~Component() = default;

			// The Entity the Component is attached to.
			LEOPPHAPI auto Entity() const -> Entity*;

		protected:
			LEOPPHAPI explicit Component(leopph::Entity* entity);

		private:
			leopph::Entity* m_Entity;
	};
}
