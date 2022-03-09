#pragma once

#include "../api/LeopphApi.hpp"
#include "../data/Poelo.hpp"


namespace leopph
{
	class Entity;

	// Components are objects that are attached to Entities to provide additional functionality.
	// They serve as a form of decoration to Entities and define their properties.
	// Subclass this to provide your own properties to Entities.
	class Component : public internal::Poelo
	{
		public:
			// The Entity the Component is attached to, or nullptr if not attached.
			[[nodiscard]] constexpr
			auto Entity() const noexcept;

			// Returns whether the Component is active.
			// Only attached active Components take part in internal calculations.
			[[nodiscard]] constexpr
			auto IsActive() const noexcept;

			// Make the Component active.
			// Only attached active Components take part in internal calculations.
			LEOPPHAPI virtual
			auto Activate() -> void;

			// Make the Component inactive.
			// Only attached active Components take part in internal calculations.
			LEOPPHAPI virtual
			auto Deactivate() -> void;

			// Attach the Component to the Entity.
			// If the Component is already attached to an Entity, it first detaches itself.
			// If the Component is already attached to the passed Entity, or the Entity is nullptr, the call is ignored.
			// Only attached active Components take part in internal calculations.
			LEOPPHAPI virtual
			auto Attach(leopph::Entity* entity) -> void;

			// Detach the Component from its Entity.
			// If the Component is not attached, the function call is silently ignored.
			// Only attached active Components take part in internal calculations.
			LEOPPHAPI virtual
			auto Detach() -> void;

			// Returns whether the Component is attached to an Entity.
			// Functionally equivalent to Entity() != nullptr.
			// Only attached active Components take part in internal calculations.
			[[nodiscard]] LEOPPHAPI
			auto IsAttached() const -> bool;

			Component(const Component& other) = delete;
			auto operator=(const Component& other) -> Component& = delete;

			Component(Component&& other) noexcept = delete;
			auto operator=(Component&& other) noexcept -> Component& = delete;

			LEOPPHAPI ~Component() override;

		protected:
			Component() = default;

		private:
			leopph::Entity* m_Entity{nullptr};
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
