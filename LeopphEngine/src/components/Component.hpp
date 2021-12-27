#pragma once

#include "../api/LeopphApi.hpp"


namespace leopph
{
	class Entity;

	// Components are objects that are attached to Entities to provide additional functionality.
	// They serve as a form of decoration to Entities and define their properties.
	// Subclass this to provide your own properties to Entities.
	class Component
	{
		public:
			// The Entity the Component is attached to.
			[[nodiscard]] constexpr auto Entity() const noexcept;

			LEOPPHAPI virtual ~Component() = default;

		protected:
			LEOPPHAPI explicit Component(leopph::Entity* entity);

			Component(const Component& other) = default;
			auto operator=(const Component& other) -> Component& = default;

			Component(Component&& other) noexcept = default;
			auto operator=(Component&& other) noexcept -> Component& = default;

		private:
			leopph::Entity* m_Entity;
	};


	constexpr auto Component::Entity() const noexcept
	{
		return m_Entity;
	}

}
