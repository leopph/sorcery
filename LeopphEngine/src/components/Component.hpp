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

			// Returns whether the Component is active.
			// Active Components take part in internal calculations.
			// Inactive Components do not.
			[[nodiscard]] constexpr auto IsActive() const noexcept;

			// Make the Component active.
			// Active Components take part in internal calculations.
			LEOPPHAPI virtual auto Activate() -> void;

			// Make the Component inactive.
			// Inactive Components do not take part in internal calculations.
			LEOPPHAPI virtual auto Deactivate() -> void;

			LEOPPHAPI virtual ~Component() = default;

		protected:
			LEOPPHAPI explicit Component(leopph::Entity* entity);

			Component(const Component& other) = default;
			auto operator=(const Component& other) -> Component& = default;

			Component(Component&& other) noexcept = default;
			auto operator=(Component&& other) noexcept -> Component& = default;

		private:
			leopph::Entity* m_Entity;
			bool m_IsActive{true};
	};


	constexpr auto Component::Entity() const noexcept
	{
		return m_Entity;
	}


	constexpr auto Component::IsActive() const noexcept
	{
		return m_IsActive;
	}
}
